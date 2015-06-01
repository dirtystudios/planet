#ifndef __application_h__
#define __application_h__

namespace app {
    struct KeyState {
        bool is_w_pressed;
        bool is_a_pressed;
        bool is_s_pressed;
        bool is_d_pressed;
        bool is_q_pressed;
        bool is_e_pressed;
    };

    struct CursorState {
        bool entered;
        double delta_x;
        double delta_y;
    };

    struct AppState {    
        KeyState key_state;
        CursorState cursor_state;
    };

    class Application {
    public:
        virtual void OnStart() = 0;
        virtual void OnFrame(const AppState* app_state, float dt) = 0;        
        virtual void OnShutdown() = 0;
    };
}

#endif