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

    // Calculate random angle within 60 degrees from the center
    float angle_offset = rand_range_f(-30.0f, 30.0f);
    float angle_radians = atan2f(direction.y, direction.x) + angle_offset;
    Vector2<float> new_target(target.x + cosf(angle_radians), target.y + sinf(angle_radians));

    // Calculate movement variation based on distance
    float distance_to_target = distance(cursor_pos, new_target);
    float movement_variation = distance_to_target * 0.05f; // Adjust as needed

    // Apply random movement variation
    new_target.x += rand_range_f(-movement_variation, movement_variation);
    new_target.y += rand_range_f(-movement_variation, movement_variation);

    // Smoothly move the cursor towards the new target
    Vector2<float> predicted_position(lerpWithEase(cursor_pos.x, new_target.x, t), lerpWithEase(cursor_pos.y, new_target.y, t));
    move_mouse_to(predicted_position.x, predicted_position.y);
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

        // Calculate movement variation based on distance
        float distance_to_slider_ball = distance(cursor_pos, slider_ball);
        float slider_variation = distance_to_slider_ball * 0.05f; // Adjust as needed

        // Apply random variation to slider ball position
        slider_ball.x += rand_range_f(-slider_variation, slider_variation);
        slider_ball.y += rand_range_f(-slider_variation, slider_variation);

        move_mouse_to_target(slider_ball, cursor_pos, t);
    } else if (circle.type == HitObjectType::Spinner && audio_time >= circle.start_time) {
        auto &center = circle.position;
        constexpr float radius = 60.0f;
        constexpr float PI = 3.14159f;
        static float angle = .0f;
        Vector2 next_point_on_circle(center.x + radius * cosf(angle), center.y + radius * sinf(angle));

        // Calculate movement variation based on distance
        float distance_to_spinner = distance(cursor_pos, next_point_on_circle);
        float spinner_variation = distance_to_spinner * 0.05f; // Adjust as needed

        // Apply random variation to spinner position
        next_point_on_circle.x += rand_range_f(-spinner_variation, spinner_variation);
        next_point_on_circle.y += rand_range_f(-spinner_variation, spinner_variation);

        move_mouse_to_target(next_point_on_circle, cursor_pos, t);

        // Apply spinning variation
        constexpr float SPIN_VARIATION = 0.2f;
        angle += cfg_spins_per_minute / (3 * PI) * ImGui::GetIO().DeltaTime + rand_range_f(-SPIN_VARIATION, SPIN_VARIATION);
    }
}
