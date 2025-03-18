#include <jo/jo.h>
#include <string.h>

// Function prototypes
static bool load_audio_file(const char* filename, jo_sound* sound);

// Constants for menu configuration
#define NUM_MENUS 3
#define BUTTONS_PER_MENU 6
#define SAMPLE_RATE 32000
#define ARROW_OFFSET_X -92
#define ARROW_OFFSET_Y 5
#define BUTTON_SPACING 30
#define INITIAL_BUTTON_Y -80
#define NAV_BUTTON_Y 100
#define SPRITE_Z_DEPTH 500

// Global variables for menu state
static int current_menu = 0;
static int current_row = 0;
static int previous_menu = -1;  // Track menu changes
static int current_audio_loaded = -1;  // Track which audio is currently loaded

// Arrays for current menu's resources only
static int menu_button_sprites[BUTTONS_PER_MENU];
static int nav_button_sprites[NUM_MENUS];        // Keep nav buttons loaded
static int nav_highlighted_sprites[NUM_MENUS];    // Keep nav buttons loaded
static jo_pos2D button_positions[BUTTONS_PER_MENU];
static jo_pos2D nav_positions[NUM_MENUS];
static int arrow_sprite;
static jo_pos2D arrow_pos;

// Single audio handle for current selection
static jo_sound current_audio;        // Regular audio
static jo_sound current_audio_o;      // "O" version audio
static jo_sound stop_audio;

// Input delay handling
static int button_delays[4] = {0, 0, 0, 0};  // up, down, left, right
static const int DELAY_FRAMES = 20; //delay between pressed movements

// Function definition for load_audio_file
static bool load_audio_file(const char* filename, jo_sound* sound) {
    if (!jo_audio_load_pcm(filename, JoSoundMono16Bit, sound)) {
        jo_core_error("Failed to load audio: %s", filename);
        return false;
    }
    sound->sample_rate = SAMPLE_RATE;
    return true;
}

// Function to free menu resources
static void free_menu_resources(void) {
    // Reset sprites (Jo Engine handles sprite memory internally)
    for (int i = 0; i < BUTTONS_PER_MENU; i++) {
        menu_button_sprites[i] = -1;
    }
    
    // Free current audio resources
    if (current_audio_loaded >= 0) {
        jo_audio_free_pcm(&current_audio);
        jo_audio_free_pcm(&current_audio_o);
        memset(&current_audio, 0, sizeof(jo_sound));
        memset(&current_audio_o, 0, sizeof(jo_sound));
        current_audio_loaded = -1;
    }
}

// Function to load a specific audio file
static bool load_specific_audio(int menu_index, int button_index) {
    char filename[20];
    
    if (current_audio_loaded == button_index) {
        return true;  // Already loaded
    }

    // Free previous audio if any
    if (current_audio_loaded >= 0) {
        jo_audio_free_pcm(&current_audio);
        jo_audio_free_pcm(&current_audio_o);
    }

    // Clear audio structures
    memset(&current_audio, 0, sizeof(jo_sound));
    memset(&current_audio_o, 0, sizeof(jo_sound));

    // Load regular audio
    sprintf(filename, "MN%dAU%d.PCM", menu_index + 1, button_index + 1);
    if (!jo_audio_load_pcm(filename, JoSoundMono16Bit, &current_audio)) {
        jo_core_error("Failed to load audio: %s", filename);
        return false;
    }
    current_audio.sample_rate = SAMPLE_RATE;

    // Load "O" version audio
    sprintf(filename, "MN%dAU%dO.PCM", menu_index + 1, button_index + 1);
    if (!jo_audio_load_pcm(filename, JoSoundMono16Bit, &current_audio_o)) {
        jo_core_error("Failed to load audio: %s", filename);
        jo_audio_free_pcm(&current_audio);  // Free the first one if second fails
        return false;
    }
    current_audio_o.sample_rate = SAMPLE_RATE;

    current_audio_loaded = button_index;
    return true;
}

// Function to load menu resources
static bool load_menu_resources(int menu_index) {
    char filename[20];
    
    // Free previous menu resources
    free_menu_resources();
    
    // Load menu button sprites
    for (int btn = 0; btn < BUTTONS_PER_MENU; btn++) {
        sprintf(filename, "MN%dB%d.BIN", menu_index + 1, btn + 1);
        int sprite_id = jo_sprite_add_bin(NULL, filename, JO_COLOR_Green);
        if (sprite_id < 0) {
            jo_core_error("Failed to load sprite: %s", filename);
            return false;
        }
        menu_button_sprites[btn] = sprite_id;
    }

    // Set up button positions for current menu
    for (int btn = 0; btn < BUTTONS_PER_MENU; btn++) {
        button_positions[btn].x = 0;
        button_positions[btn].y = INITIAL_BUTTON_Y + (btn * BUTTON_SPACING);
    }

    // Load audio for initial selection
    return load_specific_audio(menu_index, 0);
}

void my_background(void) {
    jo_img bg;
    bg.data = NULL;
    if (!jo_bin_loader(&bg, NULL, "BG.BIN", JO_COLOR_Transparent)) {
        jo_core_error("Failed to load background!");
        return;
    }
    jo_set_background_sprite(&bg, 0, 0);
    jo_free_img(&bg);
}

void init_menu(void) {
    char filename[20];
    
    // Initialize state
    current_menu = 0;
    current_row = 0;
    previous_menu = -1;
    current_audio_loaded = -1;

    // Load arrow sprite
    if ((arrow_sprite = jo_sprite_add_bin(NULL, "ARW.BIN", JO_COLOR_Green)) < 0) {
        jo_core_error("Failed to load arrow sprite!");
        return;
    }

    // Load navigation buttons (keep these loaded as they're small)
    for (int i = 0; i < NUM_MENUS; i++) {
        sprintf(filename, "NVB%d.BIN", i + 1);
        nav_button_sprites[i] = jo_sprite_add_bin(NULL, filename, JO_COLOR_Green);
        
        sprintf(filename, "NVB%dH.BIN", i + 1);
        nav_highlighted_sprites[i] = jo_sprite_add_bin(NULL, filename, JO_COLOR_Green);
    }

    // Load stop audio (keep this loaded as it's needed throughout)
    memset(&stop_audio, 0, sizeof(jo_sound));
    load_audio_file("AUSTOP.PCM", &stop_audio);

    // Set up navigation button positions
    for (int i = 0; i < NUM_MENUS; i++) {
        nav_positions[i].x = -100 + (i * 100);
        nav_positions[i].y = NAV_BUTTON_Y;
    }

    // Load initial menu (Menu 1)
    load_menu_resources(0);
}

void update_menu(void) {
    // Handle vertical movement
    if (jo_is_pad1_key_pressed(JO_KEY_UP) && button_delays[0] == 0) {
        if (current_row > 0) {
            current_row--;
            load_specific_audio(current_menu, current_row);
        }
        button_delays[0] = DELAY_FRAMES;
    }
    if (jo_is_pad1_key_pressed(JO_KEY_DOWN) && button_delays[1] == 0) {
        if (current_row < BUTTONS_PER_MENU - 1) {
            current_row++;
            load_specific_audio(current_menu, current_row);
        }
        button_delays[1] = DELAY_FRAMES;
    }

    // Menu switching with resource management
    int new_menu = current_menu;
    if (jo_is_pad1_key_pressed(JO_KEY_X)) new_menu = 0;
    if (jo_is_pad1_key_pressed(JO_KEY_Y)) new_menu = 1;
    if (jo_is_pad1_key_pressed(JO_KEY_Z)) new_menu = 2;

    // If menu changed, load new resources
    if (new_menu != current_menu) {
        if (load_menu_resources(new_menu)) {
            current_menu = new_menu;
            current_row = 0;  // Reset selection when switching menus
        }
    }

    // Handle audio playback
    if (jo_is_pad1_key_pressed(JO_KEY_A)) {
        jo_audio_play_sound_on_channel(&current_audio, 0);
    }
    if (jo_is_pad1_key_pressed(JO_KEY_B)) {
        jo_audio_play_sound_on_channel(&current_audio_o, 0);
    }
    if (jo_is_pad1_key_pressed(JO_KEY_START)) {
        jo_audio_play_sound_on_channel(&stop_audio, 0);
    }

    // Update arrow position
    arrow_pos.x = button_positions[current_row].x + ARROW_OFFSET_X;
    arrow_pos.y = button_positions[current_row].y + ARROW_OFFSET_Y;

    // Update delays
    for (int i = 0; i < 4; i++) {
        if (button_delays[i] > 0) button_delays[i]--;
    }
}

void draw_menu(void) {
    // Draw current menu's buttons
    for (int btn = 0; btn < BUTTONS_PER_MENU; btn++) {
        jo_sprite_draw3D(menu_button_sprites[btn],
                        button_positions[btn].x,
                        button_positions[btn].y,
                        SPRITE_Z_DEPTH);
    }

    // Draw navigation buttons
    for (int i = 0; i < NUM_MENUS; i++) {
        jo_sprite_draw3D(i == current_menu ? nav_highlighted_sprites[i] : nav_button_sprites[i],
                        nav_positions[i].x,
                        nav_positions[i].y,
                        SPRITE_Z_DEPTH);
    }

    // Draw selection arrow
    jo_sprite_draw3D(arrow_sprite, arrow_pos.x, arrow_pos.y, SPRITE_Z_DEPTH);
}

void main_game_loop(void) {
    update_menu();
    draw_menu();
}

void jo_main(void) {
    jo_core_init(JO_COLOR_Black);
    my_background();
    init_menu();
    jo_core_add_callback(main_game_loop);
    jo_core_run();
    
    // Cleanup
    free_menu_resources();
    jo_audio_free_pcm(&stop_audio);
}
