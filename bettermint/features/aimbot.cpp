#include "features/aimbot.h"
#include <cmath>
#include <cstdlib>

static float rand_range_f(float f_min, float f_max) {
    float scale = rand() / (float)RAND_MAX;
    return f_min + scale * (f_max - f_min);
}

static inline float smoothStep(float edge0, float edge1, float x) {
    float t = fmaxf(0.0, fminf(1.0, (x - edge0) / (edge1 - edge0)));
    return t * t * (3.0 - 2.0 * t);
}

static inline float easeInOutQuad(float t) {
    return t < 0.5 ? 2.0 * t * t : 1.0 - pow(-2.0 * t + 2.0, 2.0) / 2.0;
}

static inline float lerpWithEase(float a, float b, float t) {
    t = smoothStep(0.0f, 1.0f, t);
    return a + t * (b - a);
}

static constexpr float DEAD_ZONE_THRESHOLD = 1.0f;

template <typename T>
static inline T distance(const Vector2<T> &v1, const Vector2<T> &v2) {
    return std::sqrt(std::pow(v1.x - v2.x, 2) + std::pow(v1.y - v2.y, 2));
}

static inline Vector2<float> stableMousePosition() {
    Vector2<float> currentMousePos(.0f, .0f);
    uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
    if (!osu_manager) return currentMousePos;
    uintptr_t osu_ruleset_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_RULESET_PTR_OFFSET);
    if (!osu_ruleset_ptr) return currentMousePos;
    currentMousePos.x = *(float *)(osu_ruleset_ptr + OSU_RULESET_MOUSE_X_OFFSET);
    currentMousePos.y = *(float *)(osu_ruleset_ptr + OSU_RULESET_MOUSE_Y_OFFSET);

    static Vector2<float> lastMousePos = currentMousePos;

    if (distance(currentMousePos, lastMousePos) < DEAD_ZONE_THRESHOLD) {
        return lastMousePos;
    }

    lastMousePos = currentMousePos;
    return currentMousePos;
}

static inline void move_mouse_to_target(const Vector2<float> &target, const Vector2<float> &cursor_pos, float t) {
    Vector2<float> direction = target - cursor_pos;
    float distance = direction.length();

    // Apply smoothing
    constexpr float SMOOTHING_FACTOR = 0.8f;
    float smoothing = pow(SMOOTHING_FACTOR, t);

    // Calculate smoothed movement
    Vector2<float> smoothed_direction = direction * (1.0f - smoothing);
    Vector2<float> new_cursor_pos = cursor_pos + smoothed_direction;

    // Apply random variation
    constexpr float MOVEMENT_VARIATION = 1.5f; // Adjust as needed
    new_cursor_pos.x += rand_range_f(-MOVEMENT_VARIATION, MOVEMENT_VARIATION);
    new_cursor_pos.y += rand_range_f(-MOVEMENT_VARIATION, MOVEMENT_VARIATION);

    move_mouse_to(new_cursor_pos.x, new_cursor_pos.y);
}

void update_aimbot(Circle &circle, const int32_t audio_time) {
    if (!cfg_aimbot_lock)
        return;

    float t = cfg_fraction_modifier * ImGui::GetIO().DeltaTime;
    Vector2<float> cursor_pos = stableMousePosition();

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

        // Apply random variation
        constexpr float SLIDER_VARIATION = 5.0f;
        slider_ball.x += rand_range_f(-SLIDER_VARIATION, SLIDER_VARIATION);
        slider_ball.y += rand_range_f(-SLIDER_VARIATION, SLIDER_VARIATION);

        move_mouse_to_target(slider_ball, cursor_pos, t);
    } else if (circle.type == HitObjectType::Spinner && audio_time >= circle.start_time) {
        auto &center = circle.position;
        constexpr float radius = 60.0f;
        constexpr float PI = 3.14159f;
        static float angle = .0f;
        Vector2 next_point_on_circle(center.x + radius * cosf(angle), center.y + radius * sinf(angle));

        // Apply random variation
        constexpr float SPINNER_VARIATION = 10.0f;
        next_point_on_circle.x += rand_range_f(-SPINNER_VARIATION, SPINNER_VARIATION);
        next_point_on_circle.y += rand_range_f(-SPINNER_VARIATION, SPINNER_VARIATION);

        move_mouse_to_target(next_point_on_circle, cursor_pos, t);

        // Apply spinning variation
        constexpr float SPIN_VARIATION = 0.1f;
        angle += cfg_spins_per_minute / (3 * PI) * ImGui::GetIO().DeltaTime + rand_range_f(-SPIN_VARIATION, SPIN_VARIATION);
    }
}
