#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct SDLApplication {
    SDL_Window* window = nullptr;
    const bool* keystate = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* playerTexture = nullptr;
    float squareX = 0.0f;
    float squareY = 0.0f;
    float squareS = 10.0f;
    float circleX = 0.0f;
    float circleY = 0.0f;
    float circleS = 10.0f;
    float tri2Y = 0.0f;
    float ballX = 50.0f;
    float ballY = 350.0f;

    int LeftWindowY = 0;
    int Hit = 0;
    bool wasColliding = false;  // Setting this to count the collcision outside frame iterator



    // Audio: audio
    SDL_AudioStream* audio = nullptr;
    Uint8* wavData = nullptr;
    Uint32 wavLen = 0;
};

void DrawCircle(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    for (float y = -radius; y <= radius; y++)
    {
        for (float x = -radius; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                SDL_RenderPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

// Called once at startup (replaces the code before your while loop)
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDLApplication* app = new SDLApplication();

    app->window = SDL_CreateWindow("Ibrahim - SDL3", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!app->window) {
        SDL_Log("CreateWindow failed: %s", SDL_GetError());
        delete app;
        return SDL_APP_FAILURE;
    }
    app->keystate = SDL_GetKeyboardState(nullptr);

    *appstate = app;               // SDL hands this pointer back to the other callbacks

    // Rener the Window
    app->renderer = SDL_CreateRenderer(app->window, nullptr);
    if (!app->renderer) {
        SDL_Log("CreateRenderer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Audio: build absolute path next to the .exe and load Goblins_Dance.wav
    char wavPath[512];
    SDL_snprintf(wavPath, sizeof(wavPath), "%sGoblins_Dance.wav", SDL_GetBasePath());
    SDL_Log("Loading audio: %s", wavPath);

    SDL_AudioSpec spec;
    if (!SDL_LoadWAV(wavPath, &spec, &app->wavData, &app->wavLen)) {
        SDL_Log("LoadWAV failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Audio: open a stream on the default playback device in the WAV's format
    app->audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &spec, nullptr, nullptr);
    if (!app->audio) {
        SDL_Log("OpenAudioDeviceStream failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

//Player texture: build absolute path next to the .exe and load player.png
    char playerTexturePath[512];
    SDL_snprintf(playerTexturePath, sizeof(playerTexturePath), "%sassets/player.png", SDL_GetBasePath());
    SDL_Log("Loading texture: %s", playerTexturePath);

    app->playerTexture = IMG_LoadTexture(app->renderer, playerTexturePath);
    if (!app->playerTexture) {
        SDL_Log("IMG_LoadTexture failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    // Audio: unpause and play once as the window opens
    SDL_ResumeAudioStreamDevice(app->audio);
    SDL_PutAudioStreamData(app->audio, app->wavData, app->wavLen);

    return SDL_APP_CONTINUE;
}


// Called once per event (replaces the inner SDL_PollEvent loop)
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {

    SDLApplication* app = static_cast<SDLApplication*>(appstate);



    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;    // exit main loop, then SDL_AppQuit runs
    }
    else if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_Log("A Key was pressed: %d", event->key.key);
        switch (event->key.scancode) {
        case SDL_SCANCODE_R: SDL_Log("SDL_SCANCODE_R Key was pressed"); break;
        case SDL_SCANCODE_L: SDL_Log("SDL_SCANCODE_L Key was pressed"); break;
        default: break;
        }
        // Chords are checked with the key state array, not a case label
        if (app->keystate[SDL_SCANCODE_A] && app->keystate[SDL_SCANCODE_S]) {
            SDL_Log("A and S Key was pressed");
        }
        else if (app->keystate[SDL_SCANCODE_DOWN]) {
            app->ballY += 10.0f;  // Move the ball down by 10 pixels
            SDL_Log("SDL_SCANCODE_DOWN was pressed");
        }
        else if (app->keystate[SDL_SCANCODE_UP]) {
            app->ballY -= 10.0f;  // Move the ball UP by 10 pixels
            SDL_Log("SDL_SCANCODE_UP was pressed");
        }
        else if (app->keystate[SDL_SCANCODE_LEFT]) {
            app->ballX -= 10.0f;  // Move the ball LEFT by 10 pixels
            SDL_Log("SDL_SCANCODE_LEFT was pressed");


        }
        else if (app->keystate[SDL_SCANCODE_RIGHT]) {
            app->ballX += 10.0f;  // Move the ball RIGHT by 10 pixels
            SDL_Log("SDL_SCANCODE_RIGHT was pressed");

        }

    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION) {
        SDL_Log("x, y: %f,%f", event->motion.x, event->motion.y);
    }


    return SDL_APP_CONTINUE;
}

// Called once per frame (replaces the "Application / Game Logic" section)
SDL_AppResult SDL_AppIterate(void* appstate) {
    SDLApplication* app = static_cast<SDLApplication*>(appstate);
    (void)app;
    // Application / Game Logic

    //Debug Text


    // Create Triangle 
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);
    
    // Adding some text 
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);  /* white, full alpha */
    SDL_RenderDebugText(app->renderer, 400, 40, "Ibrahim Testing Text!");
    SDL_RenderDebugTextFormat(app->renderer, 400, 60, "Triangle Left The Window: %d", app->LeftWindowY);
    SDL_RenderDebugTextFormat(app->renderer, 400, 80, "Enemey Ball Hit Window: %d", app->Hit);


    SDL_Vertex tri[3] = {
        { {60.0f,  40.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0, 0} },  // top    � red
        { {60.0f, 200.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0, 0} },  // left   � green
        { {260.0f, 200.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0, 0} },  // right  � blue
    };
    SDL_RenderGeometry(app->renderer, nullptr, tri, 3, nullptr, 0);


    // Another one opostie side :) and it is moving down 

    SDL_Vertex tri2[3] = {
    { {470.0f,  40.0f + app->tri2Y}, {1.0f, 0.0f, 0.0f, 1.0f}, {0, 0} },  // top    � red
    { {270.0f, 200.0f + app->tri2Y}, {0.0f, 0.0f, 1.0f, 1.0f}, {0, 0} },  // left   � green
    { {470.0f, 200.0f + app->tri2Y}, {0.0f, 1.0f, 1.0f, 1.0f}, {0, 0} },  // right  � blue
    };

    SDL_RenderGeometry(app->renderer, nullptr, tri2, 3, nullptr, 0);

    app->tri2Y += 0.05f;

    if (app->tri2Y > WINDOW_HEIGHT) {
        app->tri2Y = -220.0f;   // wrap: triangle is 200 wide -220.0f
        app->LeftWindowY += 1;
    }

    
    // Differetn init Method for the triangle
    SDL_Vertex tri3[3];

    tri3[0].position.x = 600.0f;
    tri3[0].position.y = 200.0f;    
    tri3[0].color.r = 1.0f;
    tri3[0].color.g = 1.0f;
    tri3[0].color.b = 0.0f;
    tri3[0].color.a = 1.0f;

    tri3[1].position.x = 700.0f;
    tri3[1].position.y = 300.0f;
    tri3[1] .color.r = 0.0f;
    tri3[1].color.g = 1.0f;
    tri3[1].color.b = 0.0f;
    tri3[1].color.a = 1.0f;

    tri3[2].position.x = 500.0f;
    tri3[2].position.y = 300.0f;
    tri3[2].color.r = 0.0f;
    tri3[2].color.g = 0.0f;
    tri3[2].color.b = 1.0f;
    tri3[2].color.a = 1.0f;

    SDL_RenderGeometry(app->renderer, nullptr, tri3, 3, nullptr, 0);

    // Draw Door

    SDL_SetRenderDrawColor(app->renderer, 61, 0, 0, 255);   // yellow (alpha must be 255 to be visible)
    SDL_FRect door1 = { 500.0f, 300.0f, 200.0f, 250.0f };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &door1);


    SDL_SetRenderDrawColor(app->renderer, 196, 168, 0, 255);   // yellow (alpha must be 255 to be visible)
    SDL_FRect door2 = { 575.0f, 450.0f, 50.0f, 100.0f };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &door2);

    SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, 255);   // yellow (alpha must be 255 to be visible)
    SDL_FRect doorWindow = { 570.0f, 350.0f, 70.0f, 70.0f };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &doorWindow);

    // Moving part

    SDL_SetRenderDrawColor(app->renderer, 0, 222, 0, 255);   // yellow (alpha must be 255 to be visible)
    SDL_FRect Ball = { app->ballX, app->ballY, 30.0f, 30.0f };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &Ball);

    // Collision Detection between the moving ball and the door window
    if (SDL_HasRectIntersectionFloat(&Ball, &doorWindow))
    {
        SDL_Log("Collision!\n");
        if (!app->wasColliding)
        {
            app->Hit += 1;
            app->wasColliding = true;
        }
    }else{app->wasColliding = false;}



    // Wrapping moving element to windows size

    if (Ball.x > WINDOW_WIDTH) {
        app->ballX = -30.0f;  // Reset to the left side of the window
    }
    else if (Ball.x < -30.0f) {
        app->ballX = WINDOW_WIDTH;  // Reset to the right side of the window
    }else if (Ball.y > WINDOW_HEIGHT) {
        app->ballY = -30.0f;  // Reset to the top side of the window
    }
    else if (Ball.y < -30.0f) {
        app->ballY = WINDOW_HEIGHT;  // Reset to the bottom side of the window
    }

    // Move the secon traingle 



    // Render Rectangle
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);   // white
    SDL_FRect rect = { 60.0f, 205.0f, 200.0f, 30.0f };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &rect);

    // Render Rectangle
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 0, 255);   // yellow (alpha must be 255 to be visible)
    SDL_FRect rect2 = { 60.0f, 240.0f, 200.0f, 30.0f };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &rect2);


    //Square miving 

    app->squareX += 2.0f;                       // move 2 px per frame
    app->squareY += 2.0f;                       // move 2 px per frame


    if (app->squareX > WINDOW_WIDTH) app->squareX = -20.0f;   // wrap when off the right edge
    if (app->squareY > WINDOW_HEIGHT) app->squareY = -20.0f;   // wrap when off the Bottom edge


    //Square 

    SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, 255);   // Color
    SDL_FRect square = { app->squareX, app->squareY, app->squareS, app->squareS };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &square);

    //Circle miving 

    app->circleX += 10.0f;                       // move 2 px per frame
    app->circleY += 15.0f;                       // move 2 px per frame

    if (app->circleX > 800.0f) app->circleX = -20.0f;   // wrap when off the right edge
    if (app->circleY > 600.0f) app->circleY = -20.0f;   // wrap when off the Bottom edge


    //Circle 

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 255, 255);   // Color
    SDL_FRect circle = { app->circleX, app->circleY, app->circleS, app->circleS };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &circle);


    // Real Circle
    
    SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, 255);
    DrawCircle(app->renderer, 50, 500, 10);



   static SDL_FRect player{
    150.0f,
    350.0f,
    64.0f,
    64.0f
};

player.x +=0.1f; // Move the player to the right by 0.1 pixels per frame

SDL_RenderTexture(
    app->renderer,
    app->playerTexture,
    nullptr,
    &player
);





    // Show the trame
    SDL_RenderPresent(app->renderer);


    return SDL_APP_CONTINUE;
}

// Called once at shutdown (replaces SDL_Quit + cleanup)
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDLApplication* app = static_cast<SDLApplication*>(appstate);
    if (app) {
        SDL_DestroyAudioStream(app->audio);   // NEW
        SDL_free(app->wavData);               // NEW
        SDL_DestroyTexture(app->playerTexture);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }
    // SDL_Quit() is called automatically by SDL after this returns
}
