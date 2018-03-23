render_context_type* render_init(void* machine_context, void* process_context);
void init_SDL_surface(render_context_type* rc);
void update_sdl_surface_74x(render_context_type* rc);
void update_sdl_surface_2x(render_context_type* rc);
int video_output(render_context_type* rc);
int resizeWindow(render_context_type* rc, int width, int height);
void render_done(render_context_type* rc);
