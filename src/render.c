#include "render.h"

#include <math.h>


bool isbounds(Map* m, int x, int y) {
    if (x < 0 || y < 0) {
        return 0;
    }

    if (x >= m->x_size || y >= m->y_size) {
        return 0;
    }

    return 1;
}


void rendercast(Camera* cam, Map* m, Frame* f) {
    int mx,my,dof; double rx,ry,ra,xo,yo,disV,disH;

    ra = fix_angle(cam->angle - ((double)cam->rays * cam->step) / 2.0f);

    double px = cam->pos[0];
    double py = cam->pos[1];
    double eye_z = 0.5 + cam->pos[2];
    double proj_scale = (double)f->height;

    int depth = (int)ceil(cam->max_distance);
    if (depth < 1) {
        depth = 1;
    }

    for (uint32_t r = 0; r < cam->rays && r < (uint32_t)f->width; r++) {

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
                double dx = rx - px;
                double dy = ry - py;
                dof = depth;
                disV = sqrt(dx * dx + dy * dy);
                continue;
            }             
            if(m->chunk_map[mx][my].blocks[0]){ //test hit
                double dx = rx - px;
                double dy = ry - py;
                dof = depth;
                disV = sqrt(dx * dx + dy * dy);
                colorV = get_color((Types)m->chunk_map[mx][my].blocks[0]);
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
                double dx = rx - px;
                double dy = ry - py;
                dof = depth;
                disH = sqrt(dx * dx + dy * dy);
                continue;
            }    

            if(m->chunk_map[mx][my].blocks[0]){ //test hit
                double dx = rx - px;
                double dy = ry - py;
                dof = depth;
                disH = sqrt(dx * dx + dy * dy);
                colorH = get_color((Types)m->chunk_map[mx][my].blocks[0]);
            }
            else{ rx += xo; ry += yo; dof += 1;}
        }

        bool vertical_hit = disV < disH;
        double dist = vertical_hit ? disV : disH;
        uint32_t color = vertical_hit ? colorV : colorH;
        int top = (f->height >> 1);
        int bottom = top;
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

            top    = CLAMP(top,    0, (int)f->height);
            bottom = CLAMP(bottom, 0, (int)f->height);

            for (int i = top; i < bottom; i++) {
                f->pixels[i * f->width + (int)r] = color;
            }
        }

        for (int i = 0; i < top; i++) {
            f->pixels[i * f->width + (int)r] = 0xFFAAAAFF;
        }

        for (int i = bottom; i < f->height; i++) {
            uint8_t intense = 0x30 + (uint8_t)(0.2 * (double)i) ;
            uint32_t color = (intense << 16) | (intense << 8) | (intense);
            f->pixels[i * f->width + (int)r] = color;
        }

        ra += cam->step;
        fix_angle_inplace(&ra);
    }
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


}




