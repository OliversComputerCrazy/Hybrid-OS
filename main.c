#include <stdlib.h>
#include <wayland-server.h>

#include <wlr/backend.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/util/log.h>
#include <wlr/util/box.h>
#include <wlr/types/wlr_output_damage.h>

struct compositor_state {
    struct wl_display *display;
    struct wlr_backend *backend;
    struct wlr_compositor *compositor;
    struct wlr_scene *scene;
};

static void new_output_notify(struct wl_listener *listener, void *data) {
    struct wlr_output *output = data;
    wlr_output_enable(output, true);
    if (!wlr_output_commit(output)) {
        wlr_log(WLR_ERROR, "Failed to commit output");
    }
}

static void render_frame(struct wlr_output *output, struct wlr_renderer *renderer) {
    struct wlr_box bar = {
        .x = 0,
        .y = output->height - 50,
        .width = output->width,
        .height = 50
    };

    // Colors must be float[4]
    float clear_color[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    float bar_color[4]   = {1.0f, 0.0f, 0.0f, 1.0f};

    wlr_renderer_begin(renderer, output->width, output->height);
    wlr_renderer_clear(renderer, clear_color);
    wlr_render_rect(renderer, &bar, bar_color, NULL); // NULL = default projection
    wlr_output_render_software_cursors(output, NULL);
    wlr_output_commit(output);
}

int main() {
    wlr_log_init(WLR_DEBUG, NULL);

    struct compositor_state state;
    state.display = wl_display_create();
    state.backend = wlr_backend_autocreate(state.display);
    if (!state.backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        return 1;
    }

    state.compositor = wlr_compositor_create(state.display, NULL);
    if (!state.compositor) {
        wlr_log(WLR_ERROR, "Failed to create compositor");
        return 1;
    }

    state.scene = wlr_scene_create();

    // Output listener
    struct wl_listener output_listener;
    output_listener.notify = new_output_notify;
    wl_signal_add(&state.backend->events.new_output, &output_listener);

    if (!wlr_backend_start(state.backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return 1;
    }

    wlr_log(WLR_INFO, "Running compositor");
    wl_display_run(state.display);

    wl_display_destroy(state.display);
    return 0;
}

