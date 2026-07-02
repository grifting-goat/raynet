#include "render.h"

#include <math.h>
#include <stdio.h>

#define RAYCAST_MAX_THREADS 64


typedef struct {
    Camera* cam;
    Map* map;
    Frame* frame;
    uint32_t ray_start;
    uint32_t ray_end;

} RenderThreadParams;


uint32_t texture_x(double offset, Texture* tex) {
    if (tex == NULL || tex->width == 0) {
        return 0;
    }

    offset -= floor(offset);
    if (offset < 0.0) {
        offset += 1.0;
    }

    uint32_t tex_x = (uint32_t)(offset * (double)tex->width);
    if (tex_x >= tex->width) {
        tex_x = tex->width - 1;
    }

    return tex_x;
}


//might need fix
uint32_t texture_z(int real_top, int real_bottom, int screen_top, int screen_bottom, int i, Texture* tex) {
    if (tex == NULL || tex->height == 0 || real_bottom <= real_top) {
        return 0;
    }

    (void)screen_top;
    (void)screen_bottom;

    double offset = (double)(i - real_top) / (double)(real_bottom - real_top);
    if (offset < 0.0) {
        offset = 0.0;
    }
    if (offset > 1.0) {
        offset = 1.0;
    }

    uint32_t tex_z = (uint32_t)(offset * (double)tex->height);
    if (tex_z >= tex->height) {
        tex_z = tex->height - 1;
    }

    return tex_z;
}


void rendercast_range(Camera* cam, Map* m, Frame* f, uint32_t ray_start, uint32_t ray_end);

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void load_texture(Texture* t, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "could not open %s\n", path); exit(1); }

    char magic[3] = {0};
    int width, height, maxval;
    fscanf(f, "%2s", magic);
    fscanf(f, "%d %d %d", &width, &height, &maxval);
    fgetc(f);                        

    t->width = (uint32_t)width;
    t->height = (uint32_t)height;
    t->colors = malloc((size_t)width * height * sizeof(uint32_t));

    uint8_t* rgb = malloc((size_t)width * height * 3);
    fread(rgb, (size_t)width * height * 3, 1, f);
    fclose(f);
    
    for (int i = 0; i < width * height; i++) {
        uint8_t r = rgb[i*3+0], g = rgb[i*3+1], b = rgb[i*3+2];
        t->colors[i] = 0xFF000000 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    free(rgb);

    printf("loaded: %s", path);
}

void clear_buf(Texture* t) {
    t->height = 0;
    t->width = 0;
    
    free(t->colors);
    t->colors = NULL;
}



bool isbounds(Map* m, int x, int y) {
    if (x < 0 || y < 0) {
        return 0;
    }

    if (x >= m->x_size || y >= m->y_size) {
        return 0;
    }

    return 1;
}

static DWORD WINAPI rendercast_worker(LPVOID param) {
    RenderThreadParams* p = (RenderThreadParams*)param;
    rendercast_range(p->cam, p->map, p->frame, p->ray_start, p->ray_end);
    return 0;
}

void rendercast_threaded(Camera* cam, Map* m, Frame* f, uint32_t thread_count) {
    uint32_t ray_count = cam->rays;

    if (ray_count > (uint32_t)f->width) {ray_count = (uint32_t)f->width;}
    if (ray_count == 0) {return;}

    if (!thread_count) {rendercast_range(cam, m, f, 0, ray_count); return;}

    if (thread_count > ray_count) {thread_count = ray_count;}
    if (thread_count > RAYCAST_MAX_THREADS) {thread_count = RAYCAST_MAX_THREADS;}

    HANDLE threads[RAYCAST_MAX_THREADS] = {0};
    RenderThreadParams params[RAYCAST_MAX_THREADS] = {0};

    uint32_t slice = ray_count / thread_count;
    uint32_t bonus = ray_count % thread_count;
    uint32_t cursor = 0;

    for (uint32_t i = 0; i < thread_count; i++) {
        uint32_t count = slice + (i < bonus ? 1u : 0u);
        params[i].cam = cam;
        params[i].map = m;
        params[i].frame = f;
        params[i].ray_start = cursor;
        params[i].ray_end = cursor + count;
        cursor += count;

        threads[i] = CreateThread(NULL, 0, rendercast_worker, &params[i], 0, NULL);

        if (threads[i] == NULL) {
            rendercast_range(cam, m, f, params[i].ray_start, params[i].ray_end);
        }
    }

    for (uint32_t i = 0; i < thread_count; i++) {
        if (threads[i] != NULL) {
            WaitForSingleObject(threads[i], INFINITE);
            CloseHandle(threads[i]);
        }
    }
}

void rendercast_auto(Camera* cam, Map* m, Frame* f) {
    SYSTEM_INFO info = {0};
    uint32_t thread_count = 1;


    GetSystemInfo(&info);
    if (info.dwNumberOfProcessors > 1) {
        thread_count = info.dwNumberOfProcessors;
    }


    rendercast_threaded(cam, m, f, thread_count);
}



void rendercast_range(Camera* cam, Map* m, Frame* f, uint32_t ray_start, uint32_t ray_end) {
    int mx,my,dof; double rx,ry,ra,xo,yo,disV,disH;

    int texVX = 0, texHX = 0;
    Types tH = 0, tV = 0;

    ra = fix_angle(
        cam->angle - ((double)cam->rays * cam->step) / 2.0 + ((double)ray_start * cam->step)
    );

    double px = cam->pos[0];
    double py = cam->pos[1];
    double eye_z = 0.5 + cam->pos[2];
    double proj_scale = (double)f->height;

    int depth = (int)ceil(cam->max_distance);
    if (depth < 1) {
        depth = 1;
    }

    for (uint32_t r = ray_start; r < ray_end; r++) {

        //vert
        dof = 0; disV = INFINITY;

        uint32_t colorV = 0xFF000000;
        uint32_t colorH = 0xFF000000;

        double tan_ra = tan(ra);
        double cos_ra = cos(ra);
        if(cos_ra> 0.001){ 
            rx = (double)((int)px + 1);
            ry = (px-rx)*tan_ra+py; 
            xo = 1;
            yo = -xo*tan_ra;
        }
        else if(cos_ra<-0.001){ //looking right
            rx = (double)((int)px) - 0.0001;
            ry = (px-rx)*tan_ra+py; 
            xo = -1;
            yo = -xo*tan_ra;
        } 
        else { rx = px; ry = py; dof = depth;}

        while(dof<depth) { 
            mx = (int)(rx); 
            my = (int)(ry);
            if (!isbounds(m, mx, my)) {
                dof = depth;
                //double dx = rx - px;
                //double dy = ry - py;
                //disV = sqrt(dx * dx + dy * dy);
                continue;
            }             
            if(m->chunk_map[mx][my].blocks[0]){ //test hit
                double dx = rx - px;
                double dy = ry - py;
                dof = depth;
                disV = sqrt(dx * dx + dy * dy);
                tV = (Types)m->chunk_map[mx][my].blocks[0];
                texVX = texture_x(ry - floor(ry), &cam->wall_tex[tV]);
            }//hit
            else{ rx += xo; ry += yo; dof += 1;}                                              
        } 

        //hori
        dof = 0; disH = INFINITY;
        tan_ra = 1.0/tan_ra;
        double sin_ra = sin(ra);
        if(sin_ra > 0.001){
            ry = (double)((int)py) - 0.0001;
            rx = (py-ry)*tan_ra+px;
            yo = -1;
            xo = -yo*tan_ra;
        }
        else if(sin_ra < -0.001){
            ry = (double)((int)py + 1);
            rx = (py-ry)*tan_ra+px;
            yo = 1;
            xo = -yo*tan_ra;
        }
        else { rx = px; ry = py; dof = depth;}

        while(dof<depth) {
            mx = (int)(rx);
            my = (int)(ry);

            if (!isbounds(m, mx, my)) {
                dof = depth;
                //double dx = rx - px;
                //double dy = ry - py;
                //disH = sqrt(dx * dx + dy * dy);
                continue;
            }    

            if(m->chunk_map[mx][my].blocks[0]){ //test hit
                double dx = rx - px;
                double dy = ry - py;
                dof = depth;
                disH = sqrt(dx * dx + dy * dy);

                tH = (Types)m->chunk_map[mx][my].blocks[0];
                texHX = texture_x(rx - floor(rx), &cam->wall_tex[tH]);

                //colorH = get_color((Types)m->chunk_map[mx][my].blocks[0]);
            }
            else{ rx += xo; ry += yo; dof += 1;}
        }

        bool vertical_hit = disV < disH;
        double dist = vertical_hit ? disV : disH;
        Types type = vertical_hit ? tV : tH;
        int texX = vertical_hit ? texVX : texHX;

        int top = (f->height >> 1);
        int bottom = top;
        int screen_top = top;
        int screen_bottom = bottom;
        if (isfinite(dist) && dist <= cam->max_distance) {

            double ca = fix_angle(cam->angle-ra);
            dist *= cos(ca);

            if (dist <= 0.0001) {
                ra += cam->step;
                fix_angle_inplace(&ra);
                continue;
            }

            double screen_center = (double)(f->height >> 1);
            double wall_top = screen_center - ((1.0 - eye_z) / dist) * proj_scale;
            double wall_bottom = screen_center - ((0.0 - eye_z) / dist) * proj_scale;

            top = (int)wall_top;
            bottom = (int)wall_bottom;

            screen_top = top;
            screen_bottom = bottom;

            screen_top    = CLAMP(screen_top,    0, (int)f->height);
            screen_bottom = CLAMP(screen_bottom, 0, (int)f->height);

            Texture* tex = &cam->wall_tex[type];
            if (tex->colors != NULL) {
                texX = vertical_hit ? texVX : texHX;
                for (uint32_t i = (uint32_t)screen_top; i < (uint32_t)screen_bottom; i++) {
                    uint32_t texZ = texture_z(top, bottom, screen_top, screen_bottom, (int)i, tex);
                    uint32_t color = tex->colors[(texZ * tex->width) + texX];
                    f->pixels[i * f->width + (int)r] = color;
                }
            } else {
                uint32_t color = get_color(type); //remove this
                for (int i = screen_top; i < screen_bottom; i++) {
                    f->pixels[i * f->width + (int)r] = color;
                }
            }
        }

        for (int i = 0; i < screen_top; i++) {
            f->pixels[i * f->width + (int)r] = 0xFFAAAAFF;
        }

        for (int i = screen_bottom; i < f->height; i++) {
            uint8_t intense = 0x30 + (uint8_t)(0.2 * (double)i) ;
            uint32_t color = (intense << 16) | (intense << 8) | (intense);
            f->pixels[i * f->width + (int)r] = color;
        }

        ra += cam->step;
        fix_angle_inplace(&ra);
    }
}














void rendercast(Camera* cam, Map* m, Frame* f) {
    rendercast_auto(cam, m, f);
}






HitList raycast(Camera* cam, Map* m, double angle) {
    double max_distance = cam->max_distance;
    double step = cam->step;

    HitList hit_list = {0};

    double len = 0.0f;
    uint8_t collis = 0;

    while (len < max_distance && collis < MAX_DEPTH) {

        //test for collision

        //if collision add to hitlist

        len += step;
    }

    return hit_list;
}


void init_camera(Camera* cam, double fov, double max_distance, uint32_t rays) {
    cam->angle = 0.0;
    cam->fov = fov * (PI / 180.0);
    cam->max_distance = max_distance;
    cam->rays = rays;
    cam->step = cam->fov / rays;

    cam->pos[0] = 0.0;
    cam->pos[1] = 0.0;
    cam->pos[2] = 0.0;

    Texture brick;
    load_texture(&brick, "resources/brick.ppm");

    Texture dirt;
    load_texture(&dirt, "resources/dirt.ppm");

    Texture obby;
    load_texture(&obby, "resources/obby.ppm");

    cam->wall_tex[0] = brick;   
    cam->wall_tex[1] = brick;
    cam->wall_tex[2] = dirt;
    cam->wall_tex[3] = obby;

}




