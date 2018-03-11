render_context_type* render_init(void* machine_context, void* process_context);
void show_frame(render_context_type* rc);
int video_output(render_context_type* rc);
int resizeWindow(render_context_type* rc, int width, int height);
void render_done(render_context_type* rc);
