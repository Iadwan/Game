#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <string>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Frame limiter target: caps SDL_AppIterate to this many calls/second (see bottom of SDL_AppIterate).
constexpr int TARGET_FPS = 60;
constexpr Uint64 TARGET_FRAME_MS = 1000 / TARGET_FPS;

// Builds an absolute path to a file in the assets/ folder next to the executable.
// Works on both Windows and macOS regardless of the process's current directory.
static std::string AssetPath(const char* filename)
{
    static const std::string basePath = []() {
        const char* p = SDL_GetBasePath();
        return p ? std::string(p) : std::string();
    }();
    return basePath + "assets/" + filename;
}


struct Sprite
{
    SDL_Texture* mTexture = nullptr;
    SDL_FRect mDst{ 125.0f, 400.0f, 50.0f, 50.0f };

    int frameX = 0;
    int frameY = 0;

    Sprite() = default;

    void Load(SDL_Renderer* r, const std::string& filename)
    {
        SDL_Surface* surface = SDL_LoadPNG(filename.c_str());

        if (!surface)
        {
            std::cout << "SDL_LoadPNG failed: "
                << SDL_GetError() << '\n';
            return;
        }

        mTexture = SDL_CreateTextureFromSurface(r, surface);

        SDL_DestroySurface(surface);
    }

    ~Sprite()
    {
        SDL_DestroyTexture(mTexture);
    }

    void Render(SDL_Renderer* r)
    {
        SDL_FRect srcRect =
        {
            frameX * 412.0f,
            frameY * 318.0f,
            412.0f,
            318.0f
        };

        SDL_RenderTexture(
            r,
            mTexture,
            &srcRect,
            &mDst
        );
    }
};



struct SDLApplication {
    SDL_Window* window = nullptr;
    const bool* keystate = nullptr;
    SDL_Renderer* renderer = nullptr;

    Sprite sprite;
    int spriteFrame = 0;
    int spriteFrameTimer = 0;


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


    //spare
    SDL_Surface* spareSurface = SDL_LoadPNG(AssetPath("spare.png").c_str());

    SDL_Texture* spareTexture = nullptr;

    // Load fire Sprite
    SDL_Texture* explodeTexture = nullptr;
    int explodeFrame = 0;
    float explodeTimer = 0.0f;


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

    // Sprite init
    app->sprite.Load(app->renderer, AssetPath("508.png"));
    // Select Frame from sprite sheet
    app->sprite.frameX = 0;
    app->sprite.frameY = 0;

    // Init Explode fire 

    SDL_Surface* explodeSurface = SDL_LoadPNG(AssetPath("explode.png").c_str());

    if (!explodeSurface)
    {
        SDL_Log("Failed to load explode.bmp: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    app->explodeTexture =
        SDL_CreateTextureFromSurface(app->renderer, explodeSurface);

    SDL_DestroySurface(explodeSurface);

    if (!app->explodeTexture)
    {
        SDL_Log("Failed to create explosion texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }



    // Audio: build absolute path next to the .exe and load Goblins_Dance.wav
    char wavPath[512];
    SDL_snprintf(wavPath, sizeof(wavPath), "%s", AssetPath("Goblins_Dance.wav").c_str());
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

    // PLayer TEexture
    app->spareTexture = SDL_CreateTextureFromSurface(app->renderer, app->spareSurface);

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

    // Frame limiter start: timestamp this frame so we know how long it took at the end.
    const Uint64 frameStart = SDL_GetTicks();

    // Application / Game Logic

    //Debug Text


    // Create Triangle 
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    // Adding some text 
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);  /* white, full alpha */
    SDL_RenderDebugText(app->renderer, 400, 40, "Ibrahim Testing DebugText!");
    SDL_RenderDebugTextFormat(app->renderer, 400, 60, "Triangle Left The Window: %d", app->LeftWindowY);
    SDL_RenderDebugTextFormat(app->renderer, 400, 80, "Enemey Ball Hit Window: %d", app->Hit);

    // Make constexpr not tobe recreated every frame
    constexpr SDL_Vertex tri[3] = {
        { {60.0f,  40.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0, 0} },  // top – red
        { {60.0f, 200.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0, 0} },  // left – green
        { {260.0f, 200.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0, 0} },  // right – blue
    };
    SDL_RenderGeometry(app->renderer, nullptr, tri, 3, nullptr, 0);


    // Another one opostie side :) and it is moving down 

    SDL_Vertex tri2[3] = {
    { {470.0f,  40.0f + app->tri2Y}, {1.0f, 0.0f, 0.0f, 1.0f}, {0, 0} },  // top – red
    { {270.0f, 200.0f + app->tri2Y}, {0.0f, 0.0f, 1.0f, 1.0f}, {0, 0} },  // left – green
    { {470.0f, 200.0f + app->tri2Y}, {0.0f, 1.0f, 1.0f, 1.0f}, {0, 0} },  // right – blue
    };

    SDL_RenderGeometry(app->renderer, nullptr, tri2, 3, nullptr, 0);

    app->tri2Y += 1;  // Move the triangle down by 1 pixel each frame

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
    tri3[1].color.r = 0.0f;
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


    // PLayer Render
    // We create Geormetry for 
    static SDL_FRect spareRect = { 30.0f, 500.0f, 100.0f, 100.0f }; // x, y, w, h
    // Move player to the right 
    spareRect.x += 0.1f;
    if (spareRect.x > WINDOW_WIDTH) spareRect.x = -spareRect.w;   // wrap when off the right edge

    // Copy a portion of the texture to the current rendering target at subpixel precision.
    SDL_RenderTexture(app->renderer, app->spareTexture, nullptr, &spareRect);



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

    // Collision Detection between the moving ball and the PLayer
    if (SDL_HasRectIntersectionFloat(&Ball, &spareRect))
    {
        SDL_Log("Collision!\n");
        if (!app->wasColliding)
        {
            app->Hit += 1;
            app->wasColliding = true;
        }
    }
    else { app->wasColliding = false; }



    // Wrapping moving element to windows size

    if (Ball.x > WINDOW_WIDTH) {
        app->ballX = -30.0f;  // Reset to the left side of the window
    }
    else if (Ball.x < -30.0f) {
        app->ballX = WINDOW_WIDTH;  // Reset to the right side of the window
    }
    else if (Ball.y > WINDOW_HEIGHT) {
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


    //Square moving 

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

    if (app->circleX > WINDOW_WIDTH) app->circleX = -20.0f;   // wrap when off the right edge
    if (app->circleY > WINDOW_HEIGHT) app->circleY = -20.0f;   // wrap when off the Bottom edge


    //Circle 

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 255, 255);   // Color
    SDL_FRect circle = { app->circleX, app->circleY, app->circleS, app->circleS };         // x, y, w, h
    SDL_RenderFillRect(app->renderer, &circle);


    // Real Circle

    SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, 255);
    DrawCircle(app->renderer, 50, 500, 10);


    // spare Render

    spareRect.x += 0.1f;

    if (spareRect.x > WINDOW_WIDTH)
        spareRect.x = -spareRect.w;

    SDL_RenderTexture(app->renderer,
        app->spareTexture,
        nullptr,
        &spareRect);

    // Handle Sprite move

    app->spriteFrameTimer++;

    if (app->spriteFrameTimer >= 10)
    {
        app->spriteFrameTimer = 0;

        app->spriteFrame++;

        if (app->spriteFrame >= 6)
            app->spriteFrame = 0;
    }


    // Select frame from moving player
    switch (app->spriteFrame)
    {
    case 0:
        app->sprite.frameX = 0;
        app->sprite.frameY = 0;
        break;

    case 1:
        app->sprite.frameX = 0;
        app->sprite.frameY = 1;
        break;

    case 2:
        app->sprite.frameX = 0;
        app->sprite.frameY = 2;
        break;

    case 3:
        app->sprite.frameX = 1;
        app->sprite.frameY = 0;
        break;

    case 4:
        app->sprite.frameX = 1;
        app->sprite.frameY = 1;
        break;

    case 5:
        app->sprite.frameX = 1;
        app->sprite.frameY = 2;
        break;

    case 6:
        app->sprite.frameX = 2;
        app->sprite.frameY = 0;
        break;

    case 7:
        app->sprite.frameX = 2;
        app->sprite.frameY = 1;
        break;

    case 8:
        app->sprite.frameX = 2;
        app->sprite.frameY = 2;
        break;

    case 9:
        app->sprite.frameX = 3;
        app->sprite.frameY = 0;
        break;

    case 10:
        app->sprite.frameX = 3;
        app->sprite.frameY = 1;
        break;

    case 11:
        app->sprite.frameX = 3;
        app->sprite.frameY = 2;
        break;


    }
    
    // Explosion sprite sheet

// Explosion sprite sheet

    constexpr float FRAME_WIDTH = 400.0f;
    constexpr float FRAME_HEIGHT = 400.0f;

    int column = app->explodeFrame % 3;
    int row = app->explodeFrame / 3;

    SDL_FRect src =
    {
        column * FRAME_WIDTH,
        row * FRAME_HEIGHT,
        FRAME_WIDTH,
        FRAME_HEIGHT
    };

    SDL_FRect dest =
    {
        300.0f,
        400.0f,
        50.0f,
        50.0f
    };

    SDL_RenderTexture(
        app->renderer,
        app->explodeTexture,
        &src,
        &dest
    );


    //////////End of Explode //////////////

    // Render current frame
    app->sprite.Render(app->renderer);


    // New Explode Sprite
    float dt = 0.25f;

    app->explodeTimer += dt;

    if (app->explodeTimer >= 0.1f)
    {
        app->explodeTimer = 0.0f;

        app->explodeFrame++;

        if (app->explodeFrame >= 9)
        {
            app->explodeFrame = 0;
        }
    }


    // Show the trame
    SDL_RenderPresent(app->renderer);

    // Frame limiter: cap the loop to TARGET_FPS so movement speed stays
    // consistent instead of racing ahead on faster machines.
    // frameTime = how long this frame's work (logic + rendering) actually took.
    // If it finished early, sleep out the rest of the 1/TARGET_FPS budget.
    const Uint64 frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < TARGET_FRAME_MS)
    {
        SDL_Delay(static_cast<Uint32>(TARGET_FRAME_MS - frameTime));
    }

    return SDL_APP_CONTINUE;
}


// Called once at shutdown (replaces SDL_Quit + cleanup)
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDLApplication* app = static_cast<SDLApplication*>(appstate);
    if (app) {
        SDL_DestroyTexture(app->spareTexture);    // Destroy player
        SDL_DestroyAudioStream(app->audio);   // Destroy audio stream
        SDL_free(app->wavData);               // Destroy audio data

        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);

        delete app;
    }
    // SDL_Quit() is called automatically by SDL after this returns
}