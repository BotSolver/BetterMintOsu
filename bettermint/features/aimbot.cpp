#include "features/aimbot.h"
#include <cmath>
#include <cstdlib>

static float rand_range_f(float f_min, float f_max) {
    float scale = rand() / (float)RAND_MAX;
    return f_min + scale * (f_max - f_min);
}

static inline float easeInOutQuad(float t) {
    return t < 0.5 ? 2.0 * t * t : 1.0 - pow(-2.0 * t + 2.0, 2.0) / 2.0;
}

static inline float lerpWithEase(float a, float b, float t) {
    t = easeInOutQuad(t);
    return a + t * (b - a);
}

static inline Vector2<float> mouse_position() {
    Vector2<float> mouse_pos(.0f, .0f);
    uintptr_t osu_manager = *(uintptr_t*)(osu_manager_ptr);
    if (!osu_manager) return mouse_pos;
    uintptr_t osu_ruleset_ptr = *(uintptr_t*)(osu_manager + OSU_MANAGER_RULESET_PTR_OFFSET);
    if (!osu_ruleset_ptr) return mouse_pos;
    mouse_pos.x = *(float*)(osu_ruleset_ptr + OSU_RULESET_MOUSE_X_OFFSET);
    mouse_pos.y = *(float*)(osu_ruleset_ptr + OSU_RULESET_MOUSE_Y_OFFSET);
    return mouse_pos;
}

static inline void move_mouse_to_target(const Vector2<float> &target, const Vector2<float> &cursor_pos, float t) {
    Vector2 target_on_screen = playfield_to_screen(target);

    float movement_variation = 2.0f;
    target_on_screen.x += rand_range_f(-movement_variation, movement_variation);
    target_on_screen.y += rand_range_f(-movement_variation, movement_variation);

    Vector2 predicted_position(lerpWithEase(cursor_pos.x, target_on_screen.x, t), lerpWithEase(cursor_pos.y, target_on_screen.y, t));
    move_mouse_to(predicted_position.x, predicted_position.y);
}

void update_aimbot(Circle &circle, const int32_t audio_time) {
    if (!cfg_aimbot_lock)
        return;

    float t = cfg_fraction_modifier * ImGui::GetIO().DeltaTime;
    Vector2<float> cursor_pos = mouse_position();

    if (circle.type == HitObjectType::Circle) {
        move_mouse_to_target(circle.position, cursor_pos, t);
    } else if (circle.type == HitObjectType::Slider) {
        uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
        if (!osu_manager) return;
        uintptr_t hit_manager_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_HIT_MANAGER_OFFSET);
        if (!hit_manager_ptr) return;
        uintptr_t hit_objects_list_ptr = *(uintptr_t *)(hit_manager_ptr + OSU_HIT_MANAGER_HIT_OBJECTS_LIST_OFFSET);
        uintptr_t hit_objects_list_items_ptr = *(uintptr_t *)(hit_objects_list_ptr + 0x4);
        uintptr_t hit_object_ptr = *(uintptr_t *)(hit_objects_list_items_ptr + 0x8 + 0x4 * current_beatmap.hit_object_idx);
        uintptr_t animation_ptr = *(uintptr_t *)(hit_object_ptr + OSU_HIT_OBJECT_ANIMATION_OFFSET);
        float slider_ball_x = *(float *)(animation_ptr + OSU_ANIMATION_SLIDER_BALL_X_OFFSET);
        float slider_ball_y = *(float *)(animation_ptr + OSU_ANIMATION_SLIDER_BALL_Y_OFFSET);
        Vector2 slider_ball(slider_ball_x, slider_ball_y);

        float slider_variation = 5.0f;
        slider_ball.x += rand_range_f(-slider_variation, slider_variation);
        slider_ball.y += rand_range_f(-slider_variation, slider_variation);

        move_mouse_to_target(slider_ball, cursor_pos, t);
    } else if (circle.type == HitObjectType::Spinner && audio_time >= circle.start_time) {
        auto& center = circle.position;
        constexpr float radius = 60.0f;
        constexpr float PI = 3.14159f;
        static float angle = .0f;
        Vector2 next_point_on_circle(center.x + radius * cosf(angle), center.y + radius * sinf(angle));

        float spinner_variation = 10.0f;
        next_point_on_circle.x += rand_range_f(-spinner_variation, spinner_variation);
        next_point_on_circle.y += rand_range_f(-spinner_variation, spinner_variation);

        move_mouse_to_target(next_point_on_circle, cursor_pos, t);

        float spin_variation = 0.1f;
        angle += cfg_spins_per_minute / (3 * PI) * ImGui::GetIO().DeltaTime + rand_range_f(-spin_variation, spin_variation);
    }
}
