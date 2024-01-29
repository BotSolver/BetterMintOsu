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
        // Calculate a point around the circle
        Vector2<float> direction = circle.position - cursor_pos;
        float angle = atan2(direction.y, direction.x);
        constexpr float ANGLE_VARIATION = 60.0f * (M_PI / 180.0f); // 60 degrees in radians
        angle += rand_range_f(-ANGLE_VARIATION, ANGLE_VARIATION);

        Vector2<float> target(circle.position.x + cos(angle) * distance, circle.position.y + sin(angle) * distance);

        move_mouse_to_target(target, cursor_pos, t);
    } else if (circle.type == HitObjectType::Slider) {
        // Slider ball position
        float slider_ball_x = *(float *)(circle.animation_ptr + OSU_ANIMATION_SLIDER_BALL_X_OFFSET);
        float slider_ball_y = *(float *)(circle.animation_ptr + OSU_ANIMATION_SLIDER_BALL_Y_OFFSET);
        Vector2 slider_ball(slider_ball_x, slider_ball_y);

        // Apply random variation
        constexpr float SLIDER_VARIATION = 10.0f;
        slider_ball.x += rand_range_f(-SLIDER_VARIATION, SLIDER_VARIATION);
        slider_ball.y += rand_range_f(-SLIDER_VARIATION, SLIDER_VARIATION);

        move_mouse_to_target(slider_ball, cursor_pos, t);
    } else if (circle.type == HitObjectType::Spinner && audio_time >= circle.start_time) {
        auto &center = circle.position;
        constexpr float RADIUS = 60.0f;

        // Calculate a point around the spinner
        constexpr float ANGLE_VARIATION = 60.0f * (M_PI / 180.0f); // 60 degrees in radians
        static float angle = .0f;
        angle += cfg_spins_per_minute / (3 * M_PI) * ImGui::GetIO().DeltaTime + rand_range_f(-ANGLE_VARIATION, ANGLE_VARIATION);

        Vector2<float> next_point_on_spinner(center.x + RADIUS * cos(angle), center.y + RADIUS * sin(angle));

        // Apply random variation
        constexpr float SPINNER_VARIATION = 20.0f;
        next_point_on_spinner.x += rand_range_f(-SPINNER_VARIATION, SPINNER_VARIATION);
        next_point_on_spinner.y += rand_range_f(-SPINNER_VARIATION, SPINNER_VARIATION);

        move_mouse_to_target(next_point_on_spinner, cursor_pos, t);
    }
}
