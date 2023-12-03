#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <list>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600

struct Pipe
{
    int x;                             
    int gapY;                         
    static const int GAP_HEIGHT = 250; 
    static const int PIPE_WIDTH = 50;  
    bool passed = false;
};

struct Bird
{
    int x, y;          
    int width, height; 
    float velocity;    
    float gravity;     
    float lift;        
    float terminal_velocity;
    bool isColliding = false;
};

void ShowGameOverScreen(SDL_Renderer *renderer, TTF_Font *font, int score)
{
    SDL_Color textColor = {255, 255, 255, 255};

    std::string gameOverText = "Game Over!";
    std::string scoreText = "Your Score: " + std::to_string(score);
    std::string playAgainText = "Press SPACE to play again";

    SDL_Surface *gameOverSurface = TTF_RenderText_Solid(font, gameOverText.c_str(), textColor);
    SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    SDL_Surface *playAgainSurface = TTF_RenderText_Solid(font, playAgainText.c_str(), textColor);

    SDL_Texture *gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
    SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Texture *playAgainTexture = SDL_CreateTextureFromSurface(renderer, playAgainSurface);

    int gameOverWidth = gameOverSurface->w;
    int gameOverHeight = gameOverSurface->h;
    int scoreWidth = scoreSurface->w;
    int scoreHeight = scoreSurface->h;
    int playAgainWidth = playAgainSurface->w;
    int playAgainHeight = playAgainSurface->h;

    SDL_Rect gameOverRect = {WINDOW_WIDTH / 2 - gameOverWidth / 2, WINDOW_HEIGHT / 4, gameOverWidth, gameOverHeight};
    SDL_Rect scoreRect = {WINDOW_WIDTH / 2 - scoreWidth / 2, WINDOW_HEIGHT / 2, scoreWidth, scoreHeight};
    SDL_Rect playAgainRect = {WINDOW_WIDTH / 2 - playAgainWidth / 2, WINDOW_HEIGHT / 2 + scoreHeight + 20, playAgainWidth, playAgainHeight};

    SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    SDL_RenderCopy(renderer, playAgainTexture, NULL, &playAgainRect);

    SDL_FreeSurface(gameOverSurface);
    SDL_FreeSurface(scoreSurface);
    SDL_FreeSurface(playAgainSurface);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(scoreTexture);
    SDL_DestroyTexture(playAgainTexture);

    SDL_RenderPresent(renderer);

    bool waitingForSpace = true;
    while (waitingForSpace)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                exit(0);
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
            {
                waitingForSpace = false;
            }
        }
    }
}

void ResetGame(Bird &bird, std::list<Pipe> &pipes, int &score)
{
    bird.x = WINDOW_WIDTH / 8;
    bird.y = WINDOW_HEIGHT / 2;
    bird.velocity = 0;
    bird.isColliding = false;

    pipes.clear();

    score = 0;
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Can not initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        SDL_Log("Can not create window: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer)
    {
        SDL_Log("Can not create renderer: %s", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        SDL_Log("Can not initialize SDL_image: %s", IMG_GetError());
        return 1;
    }

    SDL_Texture *birdTexture = IMG_LoadTexture(renderer, "assets/img/bird.png");
    if (!birdTexture)
    {
        SDL_Log("Can not load bird image: %s", IMG_GetError());
        return 1;
    }

    SDL_Texture *upperPipeTexture = IMG_LoadTexture(renderer, "assets/img/pipeUpper.png");
    if (!upperPipeTexture)
    {
        SDL_Log("Can not load above pipe image: %s", IMG_GetError());
        return 1;
    }

    SDL_Texture *lowerPipeTexture = IMG_LoadTexture(renderer, "assets/img/pipeUnder.png");
    if (!lowerPipeTexture)
    {
        SDL_Log("Can not load below pipe image: %s", IMG_GetError());
        return 1;
    }

    SDL_Texture *bgTexture = IMG_LoadTexture(renderer, "assets/img/bg.jpg");
    if (!bgTexture)
    {
        SDL_Log("Can not load background image: %s", IMG_GetError());
        return 1;
    }

    if (TTF_Init() == -1)
    {
        SDL_Log("Can not initialize SDL_ttf: %s", TTF_GetError());
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("font.ttf", 24); 
    if (!font)
    {
        SDL_Log("Can not load font: %s", TTF_GetError());
        return 1;
    }

    bool isRunning = true;
    SDL_Event event;
    int score = 0;
    bool gameOver = false;

    Bird bird;
    bird.x = WINDOW_WIDTH / 8;  
    bird.y = WINDOW_HEIGHT / 2; 
    bird.width = 50;  
    bird.height = 50; 
    bird.velocity = 0;
    bird.gravity = 0.008;
    bird.lift = -2;
    bird.terminal_velocity = 6;

    std::list<Pipe> pipes;
    Uint32 lastPipeSpawnTime = SDL_GetTicks();
    const Uint32 PIPE_SPAWN_INTERVAL = 2000;

    while (isRunning)
    {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastPipeSpawnTime > PIPE_SPAWN_INTERVAL)
        {
            Pipe newPipe;
            newPipe.x = WINDOW_WIDTH;
            newPipe.gapY = (rand() % (WINDOW_HEIGHT - Pipe::GAP_HEIGHT)) + Pipe::GAP_HEIGHT / 2;
            pipes.push_back(newPipe);
            lastPipeSpawnTime = currentTime;
        }

        for (Pipe &pipe : pipes)
        {
            pipe.x -= 1; 
        }

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE))
            {
                bird.velocity += bird.lift;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect);

        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Color textColor = {255, 255, 255, 255}; 

        SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        if (!scoreSurface)
        {
            SDL_Log("Can not render text: %s", TTF_GetError());
            return 1;
        }

        SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        if (!scoreTexture)
        {
            SDL_Log("Can not create texture from rendered text: %s", SDL_GetError());
            return 1;
        }

        int scoreWidth = scoreSurface->w;
        int scoreHeight = scoreSurface->h;

        SDL_Rect scoreRect = {10, 10, scoreWidth, scoreHeight}; 
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);

        SDL_Rect birdRect = {bird.x, bird.y, bird.width, bird.height};
        SDL_RenderCopy(renderer, birdTexture, NULL, &birdRect);

        bird.velocity += bird.gravity;
        bird.y += bird.velocity;

        if (abs(bird.velocity) > abs(bird.terminal_velocity))
        {
            bird.velocity = bird.terminal_velocity;
        }

        if (bird.y < 0)
        {
            bird.y = 0;
            bird.velocity = 0;
        }
        else if (bird.y + bird.height > WINDOW_HEIGHT)
        {
            bird.y = WINDOW_HEIGHT - bird.height;
        }

        for (const Pipe &pipe : pipes)
        {
            SDL_Rect upperDest = {pipe.x, 0, Pipe::PIPE_WIDTH, pipe.gapY - Pipe::GAP_HEIGHT / 2};
            SDL_RenderCopy(renderer, upperPipeTexture, NULL, &upperDest);

            SDL_Rect lowerDest = {pipe.x, pipe.gapY + Pipe::GAP_HEIGHT / 2, Pipe::PIPE_WIDTH, WINDOW_HEIGHT - (pipe.gapY + Pipe::GAP_HEIGHT / 2)};
            SDL_RenderCopy(renderer, lowerPipeTexture, NULL, &lowerDest);
        }

        while (!pipes.empty() && pipes.front().x + Pipe::PIPE_WIDTH < 0)
        {
            pipes.pop_front();
        }

        bool hasCollided = false;
        for (Pipe &pipe : pipes)
        {
            if (!pipe.passed && bird.x > pipe.x + Pipe::PIPE_WIDTH)
            {
                score++;
                pipe.passed = true;
                std::cout << "Score: " << score << std::endl;
            }

            if (bird.x + bird.width > pipe.x && bird.x < pipe.x + Pipe::PIPE_WIDTH)
            {
                if (bird.y < pipe.gapY - Pipe::GAP_HEIGHT / 2 || bird.y + bird.height > pipe.gapY + Pipe::GAP_HEIGHT / 2)
                {
                    hasCollided = true;
                    break;
                }
            }
        }

        if (hasCollided && !bird.isColliding)
        {
            std::cout << "Game over!" << std::endl;
            bird.isColliding = true;
            gameOver = true;
            ShowGameOverScreen(renderer, font, score);
            ResetGame(bird, pipes, score);
        }
        else if (!hasCollided && gameOver)
        {
            bird.isColliding = false;
        }

        if (gameOver)
        {
            gameOver = false;
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(birdTexture);
    SDL_DestroyTexture(upperPipeTexture);
    SDL_DestroyTexture(lowerPipeTexture);
    SDL_DestroyTexture(bgTexture);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}