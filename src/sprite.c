#include "sprite.h"


Sprite create_sprite(const char* tex_path) {
    //creates a sprite which always faces the cam
    Texture tex;
    load_texture(&tex, tex_path);

    Sprite s;

    s.tex_cnt = 1;
    s.locked = true;
    s.tex = calloc(s.tex_cnt, sizeof(tex));
    s.tex[0] = tex;

    return s;
}

void clear_sprite(Sprite* s) {
    for (int i = 0; i<s->tex_cnt; i++) {
        clear_buf(&s->tex[i]);
    }

    free(s->tex);
    s->tex = NULL;
    s->tex_cnt = 0;
}