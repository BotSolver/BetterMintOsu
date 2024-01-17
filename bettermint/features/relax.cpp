#include "features/relax.h"
#include "window.h"
#include <cstdlib>

static float rand_range_f(float f_min, float f_max)
{
    float scale = rand() / (float)RAND_MAX;
    return f_min + scale * (f_max - f_min);
}

static int rand_range_i(int i_min, int i_max)
{
    return rand() % (i_max + 1 - i_min) + i_min;
}


float od_window = 5.f;
float od_window_left_offset = .0f;
float od_window_right_offset = .0f;
float od_check_ms = .0f;

float jumping_window_offset = .0f;

int wait_hitobjects_min = 10;
int wait_hitobjects_max = 25;

bool debug_relax = false;

static char current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];

void calc_od_timing()
{
    static const auto rand_range_f = [](float f_min, float f_max) -> float
    {
        float scale = rand() / (float)RAND_MAX;
        return f_min + scale * (f_max - f_min);
    };
    static const auto rand_range_i = [](int i_min, int i_max) -> int
    {
        return rand() % (i_max + 1 - i_min) + i_min;
    };
    if (cfg_relax_checks_od && (od_check_ms == .0f))
    {
        od_check_ms = rand_range_f(od_window_left_offset, od_window_right_offset);
        if (cfg_jumping_window)
        {
            static uint32_t hit_objects_passed = current_beatmap.hit_object_idx;
            static int wait_hitojects_count = rand_range_i(wait_hitobjects_min, wait_hitobjects_max);
            if (current_beatmap.hit_object_idx - hit_objects_passed >= wait_hitojects_count)
            {
                if (rand_range_i(0, 1) >= 1)
                    jumping_window_offset = rand_range_f(.1337f, od_window - od_window_left_offset);
                else
                    jumping_window_offset = -rand_range_f(.1337f, od_window_right_offset);
                hit_objects_passed = current_beatmap.hit_object_idx;
                wait_hitojects_count = rand_range_i(wait_hitobjects_min, wait_hitobjects_max);
            }
            od_check_ms += jumping_window_offset;
        }
    }
}

Vector2<float> mouse_position()
{
    Vector2<float> mouse_pos;
    uintptr_t osu_manager = *(uintptr_t *)(osu_manager_ptr);
    uintptr_t osu_ruleset_ptr = *(uintptr_t *)(osu_manager + OSU_MANAGER_RULESET_PTR_OFFSET);
    mouse_pos.x = *(float *)(osu_ruleset_ptr + OSU_RULESET_MOUSE_X_OFFSET);
    mouse_pos.y = *(float *)(osu_ruleset_ptr + OSU_RULESET_MOUSE_Y_OFFSET);

    return mouse_pos;
}

void update_relax(Circle &circle, const int32_t audio_time)
{
    static double keydown_time = 0.0;
    float holding_max_duration = 0.2f;

    if (cfg_relax_lock)
    {
        calc_od_timing();

        auto current_time = audio_time + od_check_ms;
        auto valid_timing = current_time >= circle.start_time;
        auto mouse_pos = mouse_position();
        Vector2 screen_pos = playfield_to_screen(circle.position);
        auto scalar_dist = sqrt((mouse_pos.x - screen_pos.x) * (mouse_pos.x - screen_pos.x) + (mouse_pos.y - screen_pos.y) * (mouse_pos.y - screen_pos.y));
        auto valid_position = scalar_dist <= current_beatmap.scaled_hit_object_radius;

        if (debug_relax)
        {
            ImGui::GetBackgroundDrawList()->AddCircleFilled(
                ImVec2(screen_pos.x, screen_pos.y),
                current_beatmap.scaled_hit_object_radius,
                ImColor(0, 255, 255, 100));
        }

        if (valid_timing && valid_position)
        {
            if (!circle.clicked)
            {
                float holding_max_duration = 0.2f;

                if (circle.type == HitObjectType::Slider || circle.type == HitObjectType::Spinner)
                {
                    keydown_time = ImGui::GetTime();
                    send_keyboard_input(current_click, 0);
                    FR_INFO_FMT("Relax hit %d!, %d %d", current_beatmap.hit_object_idx, circle.start_time, circle.end_time);
                }
                else
                {
                    float random_action = rand() / (float)RAND_MAX;
                    if (random_action < 0.5f)
                    {
                        float holding_duration = rand_range_f(0.0f, holding_max_duration);
                        keydown_time = ImGui::GetTime();
                        send_keyboard_input(current_click, 0);
                        FR_INFO_FMT("Relax hit %d!, %d %d", current_beatmap.hit_object_idx, circle.start_time, circle.end_time);
                    }
                }

                if (cfg_relax_style == 'a')
                {
                    current_click = rand() / (float)RAND_MAX < 0.5 ? left_click[0] : right_click[0];
                }

                circle.clicked = true;
                od_check_ms = .0f;
            }
        }
    }
    if (cfg_relax_lock && keydown_time)
    {
        if ((ImGui::GetTime() - keydown_time) * 1000.0 > holding_max_duration && circle.type != HitObjectType::Slider && circle.type != HitObjectType::Spinner)
        {
            keydown_time = 0.0;
            send_keyboard_input(current_click, KEYEVENTF_KEYUP);
        }
    }
}

void relax_on_beatmap_load()
{
    current_click = cfg_relax_style == 'a' ? right_click[0] : left_click[0];
}
