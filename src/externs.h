/* File: externs.h */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/* Purpose: extern declarations (variables and functions) */

/*
 * Note that some files have their own header files
 * (z-virt.h, z-util.h, z-form.h, term.h, random.h)
 */

extern bool initialized;

extern bool melee_hack;

/*
 * Automatically generated "variable" declarations
 */

extern int max_macrotrigger;
extern cptr macro_template;
extern cptr macro_modifier_chr;
extern cptr macro_modifier_name[MAX_MACRO_MOD];
extern cptr macro_trigger_name[MAX_MACRO_TRIG];
extern cptr macro_trigger_keycode[2][MAX_MACRO_TRIG];


extern int level_up;

/*
 *  List for auto-picker/destroyer entries
 */
extern int max_autopick;
extern int max_max_autopick;
extern autopick_type *autopick_list;

/* tables.c */
extern s16b ddd[9];
extern s16b ddx[10];
extern s16b ddy[10];
extern s16b ddx_ddd[9];
extern s16b ddy_ddd[9];
extern s16b cdd[8];
extern s16b ddx_cdd[8];
extern s16b ddy_cdd[8];
extern char hexsym[16];
extern char listsym[];
extern cptr color_char;
extern byte adj_mag_study[];
extern byte adj_mag_mana[];
extern byte adj_mag_fail[];
extern byte adj_mag_stat[];
extern byte adj_gold[];
extern byte adj_pseudo_id[];
extern byte adj_exp_gain[];
extern s16b adj_fear_m[];
extern s16b adj_stat_save_fear[];
extern s16b adj_stat_save[];
extern byte adj_int_dev[];
extern byte adj_wis_sav[];
extern byte adj_dex_dis[];
extern byte adj_int_dis[];
extern byte adj_dex_ta[];
extern byte adj_str_td[];
extern byte adj_dex_th[];
extern byte adj_str_th[];
extern byte adj_str_wgt[];
extern byte adj_str_hold[];
extern byte adj_str_dig[];
extern byte adj_str_blow[];
extern byte adj_dex_blow[];
extern byte adj_dex_safe[];
extern byte adj_con_fix[];
extern byte adj_con_mhp[];
extern byte adj_chr_chm[];
extern arena_type arena_info[MAX_ARENA_MONS + 2];
extern owner_type owners[MAX_STORES][MAX_OWNERS];
extern byte extract_energy[200];
extern player_sex sex_info[MAX_SEXES];
extern player_pact pact_info[WARLOCK_MAX];
extern magic_type technic_info[NUM_TECHNIC][32];
extern u32b fake_spell_flags[4];
extern s32b realm_choices1[];
extern s32b realm_choices2[];
extern cptr realm_names[];
extern cptr spell_names[VALID_REALM][32];
extern int chest_traps[64];
extern cptr player_title[MAX_CLASS][PY_MAX_LEVEL/5];
extern cptr color_names[16];
extern cptr stat_names[6];
extern cptr stat_names_reduced[6];
extern cptr stat_abbrev_true[6];
extern cptr stat_name_true[6];
extern cptr window_flag_desc[32];
extern option_type option_info[];
extern cptr chaos_patrons[MAX_PATRON];
extern martial_arts ma_blows[MAX_MA];
extern cptr game_inscriptions[];
extern kamae kamae_shurui[MAX_KAMAE];
extern kamae kata_shurui[MAX_KATA];
extern cptr exp_level_str[5];
extern cptr silly_attacks[MAX_SILLY_ATTACK];
extern monster_power monster_powers[MAX_MONSPELLS];
extern cptr monster_powers_short[MAX_MONSPELLS];
extern cptr ident_info[];
extern mbe_info_type mbe_info[];
extern byte feature_action_flags[FF_FLAG_MAX];

/* variable.c */
extern int game_mode;
extern cptr copyright[5];
extern byte sf_extra;
extern u32b sf_system;
extern byte z_major;
extern byte z_minor;
extern byte z_patch;
extern u32b sf_when;
extern u16b sf_lives;
extern u16b sf_saves;
extern bool arg_fiddle;
extern bool arg_wizard;
extern bool arg_sound;
extern byte arg_graphics;
extern bool arg_monochrome;
extern bool arg_force_original;
extern bool arg_force_roguelike;
extern bool arg_bigtile;
extern bool character_generated;
extern bool character_dungeon;
extern bool character_loaded;
extern bool character_saved;
extern bool character_icky;
extern bool character_xtra;
extern bool creating_savefile;
extern u32b seed_flavor;
extern u32b seed_town;
extern s16b command_cmd;
extern s16b command_arg;
extern s16b command_rep;
extern s16b command_dir;
extern s16b command_see;
extern s16b command_gap;
extern s16b command_wrk;
extern s16b command_new;
extern s16b energy_use;
extern s16b running;
extern s16b resting;
extern s16b cur_hgt;
extern s16b cur_wid;
extern s16b dun_level;
extern s16b unique_count;
extern s16b num_repro;
extern s16b num_repro_kill;
extern s16b object_level;
extern s16b monster_level;
extern s16b base_level;
extern s32b game_turn;
extern s32b game_turn_limit;
extern s32b player_turn;
extern s32b dungeon_turn;
extern s32b dungeon_turn_limit;
extern s32b old_turn;
extern s32b old_battle;
extern bool use_sound;
extern bool use_graphics;
extern bool use_bigtile;
extern s16b signal_count;
extern bool inkey_base;
extern bool inkey_xtra;
extern bool inkey_scan;
extern bool inkey_flag;
extern bool get_com_no_macros;
extern s16b coin_type;
extern bool opening_chest;
extern bool shimmer_monsters;
extern bool shimmer_objects;
extern bool repair_monsters;
extern bool repair_objects;
extern s16b inven_nxt;
extern s16b inven_cnt;
extern s16b o_max;
extern s16b o_cnt;
extern s16b m_max;
extern s16b m_cnt;
extern s16b hack_m_idx;
extern int hack_m_spell;
extern s16b hack_m_idx_ii;
extern int total_friends;
extern s32b friend_align;
extern int leaving_quest;
extern bool reinit_wilderness;
extern char summon_kin_type;
extern bool hack_mind;

/*
 * Software options (set via the '=' command).  See "tables.c"
 */

/*** Input Options ***/

extern bool rogue_like_commands;    /* Rogue-like commands */
extern bool always_pickup;    /* Pick things up by default */
extern bool carry_query_flag;    /* Prompt before picking things up */
extern bool quick_messages;    /* Activate quick messages */
extern bool command_menu;    /* Enable command selection menu */
extern bool other_query_flag;    /* Prompt for floor item selection */
extern bool use_old_target;    /* Use old target by default */
extern bool always_repeat;    /* Repeat obvious commands */
extern bool confirm_destroy;    /* Prompt for destruction of known worthless items */
extern bool confirm_wear;    /* Confirm to wear/wield known cursed items */
extern bool confirm_quest;    /* Prompt before exiting a quest level */
extern bool target_pet;    /* Allow targetting pets */

#ifdef ALLOW_EASY_OPEN
extern bool easy_open;    /* Automatically open doors */
#endif

#ifdef ALLOW_EASY_DISARM
extern bool easy_disarm;    /* Automatically disarm traps */
#endif

#ifdef ALLOW_EASY_FLOOR
extern bool easy_floor;    /* Display floor stacks in a list */
#endif

extern bool over_exert;    /* Allow casting spells when short of mana */
extern bool numpad_as_cursorkey;    /* Use numpad keys as cursor key in editor mode */


/*** Map Screen Options ***/

extern bool center_player;    /* Center map while walking (*slow*) */
extern bool center_running;    /* Centering even while running */
extern bool view_yellow_lite;    /* Use special colors for torch-lit grids */
extern bool view_bright_lite;    /* Use special colors for 'viewable' grids */
extern bool view_granite_lite;    /* Use special colors for wall grids (slow) */
extern bool view_special_lite;    /* Use special colors for floor grids (slow) */
extern bool view_perma_grids;    /* Map remembers all perma-lit grids */
extern bool view_torch_grids;    /* Map remembers all torch-lit grids */
extern bool view_unsafe_grids;    /* Map marked by detect traps */
extern bool fresh_before;    /* Flush output while continuous command */
extern bool fresh_after;    /* Flush output after monster's move */
extern bool fresh_message;    /* Flush output after every message */
extern bool hilite_player;    /* Hilite the player with the cursor */
extern bool display_path;    /* Display actual path before shooting */


/*** Text Display Options ***/

extern bool plain_descriptions;    /* Plain object descriptions */
extern bool plain_pickup;    /* Plain pickup messages(japanese only) */
extern bool always_show_list;    /* Always show list when choosing items */
extern bool depth_in_feet;    /* Show dungeon level in feet */
extern bool show_labels;    /* Show labels in object listings */
extern bool show_weights;    /* Show weights in object listings */
extern bool show_discounts;
extern bool show_item_graph;    /* Show items graphics */
extern bool equippy_chars;    /* Display 'equippy' chars */
extern bool display_food_bar;    /* Like the monster health bar, only tastier! */
extern bool display_hp_bar; /* Display player HP just like the monster health bar */
extern bool display_sp_bar; /* Display player SP just like the monster health bar */
extern bool compress_savefile;    /* Compress messages in savefiles */
extern bool abbrev_extra;    /* Describe obj's extra resistances by abbreviation */
extern bool abbrev_all;    /* Describe obj's all resistances by abbreviation */
extern bool exp_need;    /* Show the experience needed for next level */
extern bool ignore_unview;    /* Ignore whenever any monster does */
extern bool display_distance;    /* Display distance of LoS monsters in monster list */
extern bool simple_stat_display;

/*** Game-Play Options ***/

extern bool stack_force_notes;    /* Merge inscriptions when stacking */
extern bool stack_force_costs;    /* Merge discounts when stacking */
extern bool expand_list;    /* Expand the power of the list commands */
extern bool small_levels;    /* Allow unusually small dungeon levels */
extern bool always_small_levels;    /* Always create unusually small dungeon levels */
extern bool empty_levels;    /* Allow empty 'arena' levels */
extern bool bound_walls_perm;    /* Boundary walls become 'permanent wall' */
extern bool last_words;    /* Leave last words when your character dies */

#ifdef WORLD_SCORE
extern bool send_score;    /* Send score dump to the world score server */
#endif

extern bool allow_debug_opts;    /* Allow use of debug/cheat options */


/*** Disturbance Options ***/

extern bool find_ignore_stairs;    /* Run past stairs */
extern bool find_ignore_doors;    /* Run through open doors */
extern bool find_cut;    /* Run past known corners */
extern bool check_abort;    /* Check for user abort while continuous command */
extern bool flush_failure;    /* Flush input on various failures */
extern bool flush_disturb;    /* Flush input whenever disturbed */
extern bool disturb_move;    /* Disturb whenever any monster moves */
extern bool disturb_high;    /* Disturb whenever high-level monster moves */
extern bool disturb_near;    /* Disturb whenever viewable monster moves */
extern bool disturb_pets;    /* Disturb when visible pets move */
extern bool disturb_panel;    /* Disturb whenever map panel changes */
extern bool disturb_state;    /* Disturb whenever player state changes */
extern bool disturb_minor;    /* Disturb whenever boring things happen */
extern bool town_no_disturb;    /* Never disturb when a town monster moves */
extern bool ring_bell;    /* Audible bell (on errors, etc) */
extern bool disturb_trap_detect;    /* Disturb when leaving trap detected area */
extern bool alert_trap_detect;    /* Alert when leaving trap detected area */


/*** Birth Options ***/

extern bool easy_id;        /* Easy Identify */
extern bool easy_lore;      /* Easy Monster Lore */
extern bool smart_learn;    /* Monsters learn from their mistakes (*) */
extern bool smart_cheat;    /* Monsters exploit players weaknesses (*) */
extern bool no_wilderness;  /* Play without a normal wilderness */
extern bool ironman_shops;    /* Stores are permanently closed (*) */
extern bool ironman_small_levels;    /* Always create unusually small dungeon levels (*) */
extern bool ironman_downward;    /* Disable recall and use of up stairs (*) */
extern bool ironman_empty_levels;    /* Always create empty 'arena' levels (*) */
extern bool ironman_rooms;    /* Always generate very unusual rooms (*) */
extern bool ironman_nightmare;    /* Nightmare mode(it isn't even remotely fair!)(*) */
extern bool preserve_mode;    /* Preserve artifacts (*) */
extern bool autoroller;    /* Allow use of autoroller for stats (*) */
extern bool powerup_home;    /* Increase capacity of your home (*) */
extern bool allow_friendly_monster; /* Allow monsters friendly to player */
extern bool allow_hostile_monster; /* Allow monsters hostile to each other */
extern bool allow_pets; /* Allow pets: Note, this makes some classes unplayable. */
extern bool quest_unique; /* Random quests for unique monsters only */
extern bool ironman_quests; /* Random quests must be completed */
extern bool random_artifacts;
extern bool no_artifacts;
extern bool no_egos;
extern bool enable_virtues;

/*** Easy Object Auto-Destroyer ***/

extern bool destroy_items;    /* Use easy auto-destroyer */
extern bool destroy_debug;
extern bool destroy_feeling;    /* Apply auto-destroy as sense feeling */
extern bool destroy_identify;    /* Apply auto-destroy as identify an item */
extern bool leave_worth;    /* Auto-destroyer leaves known worthy items */
extern bool leave_equip;    /* Auto-destroyer leaves weapons and armour */
extern bool leave_chest;    /* Auto-destroyer leaves closed chests */
extern bool leave_wanted;    /* Auto-destroyer leaves wanted corpses */
extern bool leave_corpse;    /* Auto-destroyer leaves corpses and skeletons */
extern bool leave_junk;    /* Auto-destroyer leaves junk */
extern bool leave_special;    /* Auto-destroyer leaves items your race/class needs */

extern bool cheat_peek;
extern bool cheat_hear;
extern bool cheat_room;
extern bool cheat_xtra;
extern bool cheat_live;
extern bool cheat_save;

extern byte hitpoint_warn;
extern byte mana_warn;
extern byte delay_factor;
extern s16b autosave_freq;
extern bool autosave_t;
extern bool autosave_l;
extern bool closing_flag;

extern int py;
extern int px;
extern s16b target_who;
extern s16b target_col;
extern s16b target_row;
extern int player_uid;
extern int player_euid;
extern int player_egid;
extern char player_name[32];
extern char player_base[32];
extern char savefile[1024];
extern char savefile_base[40];
extern s16b lite_n;
extern s16b lite_y[LITE_MAX];
extern s16b lite_x[LITE_MAX];
extern s16b mon_lite_n;
extern s16b mon_lite_y[MON_LITE_MAX];
extern s16b mon_lite_x[MON_LITE_MAX];
extern s16b view_n;
extern s16b view_y[VIEW_MAX];
extern s16b view_x[VIEW_MAX];
extern s16b temp_n;
extern s16b temp_y[TEMP_MAX];
extern s16b temp_x[TEMP_MAX];
extern s16b redraw_n;
extern s16b redraw_y[REDRAW_MAX];
extern s16b redraw_x[REDRAW_MAX];
extern s16b macro__num;
extern cptr *macro__pat;
extern cptr *macro__act;
extern bool *macro__cmd;
extern char *macro__buf;
extern s16b quark__num;
extern cptr *quark__str;
extern u32b option_flag[8];
extern u32b option_mask[8];
extern u32b window_flag[8];
extern u32b window_mask[8];
extern term *angband_term[8];
extern char angband_term_name[8][16];
extern byte angband_color_table[256][4];
extern char angband_sound_name[SOUND_MAX][16];
extern cave_type *cave[MAX_HGT];
extern saved_floor_type saved_floors[MAX_SAVED_FLOORS];
extern s16b max_floor_id;
extern u32b saved_floor_file_sign;
extern object_type *o_list;
extern monster_type *m_list;

extern pack_info_t *pack_info_list;
extern s16b max_pack_info_idx;
extern s16b pack_info_free_list;
extern s16b pack_info_count;


extern s16b *mproc_list[MAX_MTIMED];
extern s16b mproc_max[MAX_MTIMED];
extern u16b max_towns;
extern town_type *town;
extern object_type *inventory;
extern s16b alloc_kind_size;
extern alloc_entry *alloc_kind_table;
extern s16b alloc_race_size;
extern alloc_entry *alloc_race_table;
extern byte misc_to_attr[256];
extern char misc_to_char[256];
extern byte tval_to_attr[128];
extern char tval_to_char[128];
extern cptr keymap_act[KEYMAP_MODES][256];
extern player_type *p_ptr;
extern player_sex *sp_ptr;
extern player_magic *mp_ptr;
extern birther previous_char;
extern room_template_t *room_info;
extern char *room_name;
extern char *room_text;
extern player_magic *m_info;
extern feature_type *f_info;
extern char *f_name;
extern char *f_tag;
extern object_kind *k_info;
extern char *k_name;
extern char *k_text;
extern artifact_type *a_info;
extern char *a_name;
extern char *a_text;
extern ego_type *e_info;
extern char *e_name;
extern char *e_text;
extern monster_race *r_info;
extern char *r_name;
extern char *r_text;
extern equip_template_ptr b_info;
extern char *b_name;
extern char *b_tag;
extern dungeon_info_type *d_info;
extern char *d_name;
extern char *d_text;
extern cptr ANGBAND_SYS;
extern cptr ANGBAND_KEYBOARD;
extern cptr ANGBAND_GRAF;
extern cptr ANGBAND_DIR;
extern cptr ANGBAND_DIR_APEX;
extern cptr ANGBAND_DIR_BONE;
extern cptr ANGBAND_DIR_DATA;
extern cptr ANGBAND_DIR_EDIT;
extern cptr ANGBAND_DIR_SCRIPT;
extern cptr ANGBAND_DIR_FILE;
extern cptr ANGBAND_DIR_HELP;
extern cptr ANGBAND_DIR_INFO;
extern cptr ANGBAND_DIR_PREF;
extern cptr ANGBAND_DIR_SAVE;
extern cptr ANGBAND_DIR_USER;
extern cptr ANGBAND_DIR_XTRA;
extern bool item_tester_full;
extern bool item_tester_no_ryoute;
extern byte item_tester_tval;
extern bool (*item_tester_hook)(object_type *o_ptr);
extern bool (*ang_sort_comp)(vptr u, vptr v, int a, int b);
extern void (*ang_sort_swap)(vptr u, vptr v, int a, int b);
extern monster_hook_type get_mon_num_hook;
extern monster_hook_type get_mon_num2_hook;
extern bool (*get_obj_num_hook)(int k_idx);
extern int  obj_drop_theme;
extern bool monk_armour_aux;
extern bool monk_notify_aux;
extern wilderness_type **wilderness;
extern building_type building[MAX_BLDG];
extern u16b max_quests;
extern byte num_random_quests;
extern u16b max_r_idx;
extern u16b max_b_idx;
extern u16b max_k_idx;
extern u16b max_room_idx;
extern u16b max_f_idx;
extern u16b max_a_idx;
extern u16b max_e_idx;
extern u16b max_d_idx;
extern u16b max_o_idx;
extern u16b max_m_idx;
extern s32b max_wild_x;
extern s32b max_wild_y;
extern quest_type *quest;
extern char quest_text[10][80];
extern int quest_text_line;
extern s16b gf_color[MAX_GF];
extern int init_flags;
extern int init_dx;
extern int init_dy;
extern const rect_t *init_exclude_rect;
extern int highscore_fd;
extern int mutant_regenerate_mod;
extern bool can_save;
extern s16b world_monster;
extern bool world_player;
extern int cap_mon;
extern int cap_mspeed;
extern int cap_hp;
extern int cap_maxhp;
extern u16b cap_nickname;
extern s16b battle_mon[4];
extern int sel_monster;
extern int battle_odds;
extern int kakekin;
extern u32b mon_odds[4];
extern int pet_t_m_idx;
extern int riding_t_m_idx;
extern s16b kubi_r_idx[MAX_KUBI];
extern s16b today_mon;
extern bool write_level;
extern u32b playtime;
extern time_t start_time;
extern int tsuri_dir;
extern bool sukekaku;
extern bool new_mane;
extern bool mon_fight;
extern bool generate_encounter;
extern cptr screen_dump;

/*** Terrain feature variables ***/
extern s16b feat_none;
extern s16b feat_floor;
extern s16b feat_glyph;
extern s16b feat_explosive_rune;
extern s16b feat_mirror;
extern s16b feat_rogue_trap1;
extern s16b feat_rogue_trap2;
extern s16b feat_rogue_trap3;
extern door_type feat_door[MAX_DOOR_TYPES];
extern s16b feat_up_stair;
extern s16b feat_down_stair;
extern s16b feat_entrance;
extern s16b feat_trap_open;
extern s16b feat_trap_armageddon;
extern s16b feat_trap_piranha;
extern s16b feat_rubble;
extern s16b feat_magma_vein;
extern s16b feat_quartz_vein;
extern s16b feat_granite;
extern s16b feat_permanent;
extern s16b feat_glass_floor;
extern s16b feat_glass_wall;
extern s16b feat_permanent_glass_wall;
extern s16b feat_pattern_start;
extern s16b feat_pattern_1;
extern s16b feat_pattern_2;
extern s16b feat_pattern_3;
extern s16b feat_pattern_4;
extern s16b feat_pattern_end;
extern s16b feat_pattern_old;
extern s16b feat_pattern_exit;
extern s16b feat_pattern_corrupted;
extern s16b feat_black_market;
extern s16b feat_town;
extern s16b feat_deep_water;
extern s16b feat_shallow_water;
extern s16b feat_deep_lava;
extern s16b feat_shallow_lava;
extern s16b feat_dirt;
extern s16b feat_grass;
extern s16b feat_flower;
extern s16b feat_brake;
extern s16b feat_tree;
extern s16b feat_mountain;
extern s16b feat_swamp;
extern s16b feat_undetected;
extern s16b feat_dark_pit;
extern s16b feat_web;

extern byte dungeon_type;
extern s16b *max_dlv;

/* Ideas for Dungeon Flags:
    [1] Address Stair Scumming by occasionally removing the wilderness entrance. (Done)
    [2] Lock dungeons until a certain event takes place (Proposed)
*/
#define DUNGEON_NO_ENTRANCE 0x0001  /* Wilderness Entrance has collapsed (stair scummer?) */
#define DUNGEON_NO_GUARDIAN 0x0002  /* Wilderness Entrance Guardian has been slain */
extern u32b *dungeon_flags;

extern s16b feat_wall_outer;
extern s16b feat_wall_inner;
extern s16b feat_wall_solid;
extern s16b floor_type[100], fill_type[100];
extern s32b now_turn;
extern bool use_menu;

/* autopick.c */
extern void autopick_load_pref(bool disp_mes);
extern errr process_autopick_file_command(char *buf);
#define AUTOPICK_COLOR_CODED 0x01
extern string_ptr autopick_line_from_entry(autopick_type *entry, int options);
extern int is_autopick(object_type *o_ptr);
extern void autopick_alter_item(int item, bool destroy);
extern void autopick_delayed_alter(void);
extern void autopick_pickup_items(cave_type *c_ptr);
extern bool autopick_autoregister(object_type *o_ptr);
extern void do_cmd_edit_autopick(void);
extern int pack_find_device(int effect);
extern int pack_find(int tval, int sval);

/* birth.c */
extern cptr realm_jouhou[VALID_REALM];
extern bool birth_hack;
extern void add_history_from_pref_line(cptr t);
extern void add_outfit(object_type *o_ptr);
extern cptr birth_get_class_desc(int i);
extern cptr birth_get_realm_desc(int i);
extern void player_birth(void);
extern void get_max_stats(void);
extern void determine_random_questor(quest_type *q_ptr);
extern void player_outfit(void);
extern int calc_exp_factor(void);

/* cave.c */
extern int distance(int y1, int x1, int y2, int x2);
extern bool is_trap(int feat);
extern bool is_known_trap(cave_type *c_ptr);
extern bool is_closed_door(int feat);
extern bool is_hidden_door(cave_type *c_ptr);
extern bool is_jammed_door(int feat);
extern bool los(int y1, int x1, int y2, int x2);
extern void update_local_illumination(int y, int x);
extern bool player_can_see_bold(int y, int x);
extern bool cave_valid_bold(int y, int x);
extern bool no_lite(void);
extern void apply_default_feat_lighting(byte f_attr[F_LIT_MAX], byte f_char[F_LIT_MAX]);
extern void map_info(int y, int x, byte *ap, char *cp, byte *tap, char *tcp);
extern void move_cursor_relative(int row, int col);
extern void print_rel(char c, byte a, int y, int x);
extern void note_spot(int y, int x);
extern void display_dungeon(void);
extern void lite_spot(int y, int x);
extern void prt_map(void);
extern void prt_path(int y, int x);
extern void display_map(int *cy, int *cx);
extern void do_cmd_view_map(void);
extern void forget_lite(void);
extern void update_lite(void);
extern void forget_view(void);
extern void update_view(void);
extern void update_mon_lite(void);
extern void clear_mon_lite(void);
extern void delayed_visual_update(void);
extern void forget_flow(void);
extern void update_flow(void);
extern int  current_flow_depth;
extern void update_smell(void);
extern void map_area(int range);
extern void wiz_lite(bool ninja);
extern void wiz_dark(void);
extern void cave_set_feat(int y, int x, int feat);
extern int conv_dungeon_feat(int newfeat);
extern int feat_state(int feat, int action);
extern void cave_alter_feat(int y, int x, int action);
extern bool is_glyph_grid(cave_type *c_ptr);
extern bool is_mon_trap_grid(cave_type *c_ptr);
extern void hit_mon_trap(int y, int x, int m_idx);
extern void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);
extern bool projectable(int y1, int x1, int y2, int x2);
extern void scatter(int *yp, int *xp, int y, int x, int d, int mode);
extern void health_track(int m_idx);
extern void monster_race_track(int r_idx);
extern void object_kind_track(int k_idx);
extern void disturb(int stop_search, int flush_output);
extern void glow_deep_lava_and_bldg(void);
extern void py_get_display_char_attr(char *c, byte *a);

/* cmd1.c */
extern void rune_sword_kill(object_type *o_ptr, monster_race *r_ptr);
extern void touch_zap_player(int m_idx);
extern bool test_hit_fire(int chance, int ac, int vis);
extern bool random_opponent(int *y, int *x);
extern bool test_hit_norm(int chance, int ac, int vis);
extern s16b critical_throw(int weight, int plus, int dam);

typedef struct critical_s {
    int mul; /* Scaled by 100 */
    int to_d;
    cptr desc;
} critical_t;
extern critical_t critical_shot(int weight, int plus);
extern critical_t critical_norm(int weight, int plus, s16b meichuu, int mode, int hand);

extern s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr, s16b hand, int mode, bool thrown);
extern void search(void);
extern void py_pickup_aux(int o_idx);
extern void carry(bool pickup);
extern bool py_attack(int y, int x, int mode);
extern bool pattern_seq(int c_y, int c_x, int n_y, int n_x);
extern bool player_can_enter(s16b feature, u16b mode);
extern bool move_player_effect(int ny, int nx, u32b mpe_mode);
extern bool trap_can_be_ignored(int feat);
extern void move_player(int dir, bool do_pickup, bool break_trap);
extern void run_step(int dir);
extern void travel_step(void);

/* cmd2.c */
extern void do_cmd_go_up(void);
extern void do_cmd_go_down(void);
extern void do_cmd_search(void);
extern void do_cmd_open(void);
extern void do_cmd_close(void);
extern void do_cmd_tunnel(void);
extern void do_cmd_disarm(void);
extern void do_cmd_bash(void);
extern void do_cmd_alter(void);
extern void do_cmd_spike(void);
extern void do_cmd_walk(bool pickup);
extern void do_cmd_stay(bool pickup);
extern void do_cmd_run(void);
extern void do_cmd_rest(void);
extern bool do_cmd_fire(void);
extern bool do_cmd_fire_aux1(int item, object_type *j_ptr); /* ammo already chosen */
extern void do_cmd_fire_aux2(int item, object_type *j_ptr, int sx, int sy, int tx, int ty); /* ammo and target already chosen */
extern void do_cmd_throw(void);
extern bool do_cmd_throw_aux(int mult, bool boomerang, int shuriken);
extern void do_cmd_travel(void);
extern void do_cmd_travel_xy(int x, int y);

/* cmd3.c */
extern void do_cmd_inven(void);
extern void do_cmd_equip(void);
extern void do_cmd_drop(void);
extern void do_cmd_destroy(void);
extern void do_cmd_inspect(void);
extern void do_cmd_uninscribe(void);
extern void do_cmd_inscribe(void);
extern void do_cmd_refill(void);
extern void do_cmd_target(void);
extern void do_cmd_look(void);
extern void do_cmd_locate(void);
extern void do_cmd_query_symbol(void);
extern void kamaenaoshi(int item);
extern bool ang_sort_comp_hook(vptr u, vptr v, int a, int b);
extern void ang_sort_swap_hook(vptr u, vptr v, int a, int b);
enum {
    MON_LIST_NORMAL = 0,
    MON_LIST_PROBING
};
extern void do_cmd_list_monsters(int mode);
extern void do_cmd_list_objects(void);
extern void fix_monster_list(void);
extern void fix_object_list(void);

/* cmd4.c */
extern cptr get_ordinal_number_suffix(int num);
extern void do_cmd_redraw(void);
extern void do_cmd_knowledge_weapon(void);
extern void do_cmd_messages(int old_now_turn);
extern void do_cmd_options_aux(int page, cptr info);
extern void do_cmd_options(void);
extern void do_cmd_pref(void);
extern void do_cmd_reload_autopick(void);
extern void do_cmd_macros(void);
extern void do_cmd_visuals(void);
extern void do_cmd_colors(void);
extern void do_cmd_note(void);
extern void do_cmd_version(void);
extern void do_cmd_feeling(void);
extern void do_cmd_save_screen(void);
extern void do_cmd_save_screen_doc(void);
extern void do_cmd_save_screen_html(void);
extern void do_cmd_save_screen_txt(void);
extern void save_screen_aux(cptr file, int format);
extern void do_cmd_knowledge_quests_completed(FILE *fff, int quest_num[]);
extern void do_cmd_knowledge_quests_failed(FILE *fff, int quest_num[]);
extern bool ang_sort_comp_quest_num(vptr u, vptr v, int a, int b);
extern void ang_sort_swap_quest_num(vptr u, vptr v, int a, int b);
extern void do_cmd_knowledge(void);
extern void plural_aux(char * Name);
extern void do_cmd_checkquest(void);
extern void do_cmd_time(void);
extern string_ptr get_tiny_screenshot(int cx, int cy);
extern string_ptr get_screenshot(void);
extern void msg_add_tiny_screenshot(int cx, int cy);

/* cmd5.c */
extern cptr spell_category_name(int tval);
extern void do_cmd_browse(void);
extern void do_cmd_study(void);
extern void do_cmd_cast(void);
extern bool rakuba(int dam, bool force);
extern bool do_riding(bool force);
extern void check_pets_num_and_align(monster_type *m_ptr, bool inc);
extern int calculate_upkeep(void);
extern void do_cmd_pet_dismiss(void);
extern void do_cmd_pet(void);
extern bool player_can_ride_aux(cave_type *c_ptr, bool now_riding);

/* cmd6.c */
extern void do_cmd_eat_food(void);
extern void do_cmd_quaff_potion(void);
extern void do_cmd_read_scroll(void);
extern void do_cmd_aim_wand(void);
extern void do_cmd_use_staff(void);
extern void do_cmd_zap_rod(void);
extern void do_cmd_activate(void);
extern void do_cmd_rerate_aux(void);
extern int life_rating(void);
extern void do_cmd_rerate(bool display);
extern void ring_of_power(int dir);
extern bool restore_mana(void);

/* devices.c */
extern int  device_calc_fail_rate(object_type *o_ptr); /*95.2% returned as 952*/
extern bool device_try(object_type *o_ptr);
extern bool device_use(object_type *o_ptr, int boost);
extern bool device_known;
extern bool device_noticed;
extern int  device_extra_power;
extern int  device_available_charges;
extern int  device_used_charges;
extern cptr do_device(object_type *o_ptr, int mode, int boost);

extern bool device_init(object_type *o_ptr, int level, int mode);
extern bool device_init_fixed(object_type *o_ptr, int effect);
extern int  device_level(object_type *o_ptr);
extern int  device_sp(object_type *o_ptr);
extern void device_decrease_sp(object_type *o_ptr, int amt);
extern void device_increase_sp(object_type *o_ptr, int amt);
extern int  device_max_sp(object_type *o_ptr);
extern bool device_is_fully_charged(object_type *o_ptr);
extern void device_regen_sp(object_type *o_ptr, int base_per_mill);
extern void device_regen_sp_aux(object_type *o_ptr, int per_mill);
extern int  device_value(object_type *o_ptr, int options);
extern void device_stats_reset(void);
extern void device_stats_on_find(object_type *o_ptr);
extern void device_stats_on_use(object_type *o_ptr, int num);
extern void device_stats_on_destroy(object_type *o_ptr);
extern void device_stats_on_purchase(object_type *o_ptr);
extern void device_stats_on_load(savefile_ptr file);
extern void device_stats_on_save(savefile_ptr file);

extern device_effect_info_ptr device_get_effect_info(int tval, int effect);
extern device_effect_info_t   wand_effect_table[];
extern device_effect_info_t   rod_effect_table[];
extern device_effect_info_t   staff_effect_table[];

extern effect_t obj_get_effect(object_type *o_ptr);
extern cptr     obj_get_effect_msg(object_type *o_ptr);
extern bool     obj_has_effect(object_type *o_ptr);
extern int      effect_calc_fail_rate(effect_t *effect_ptr);
extern bool     effect_add_random(object_type *o_ptr, int bias);

typedef bool (*effect_p)(int effect);
extern bool effect_add_random_p(object_type *o_ptr, effect_p p);

extern bool     effect_add(object_type *o_ptr, int type);
extern bool     effect_try(effect_t *effect_ptr);
extern bool     effect_use(effect_t *effect_ptr, int boost);
extern errr     effect_parse(char *line, effect_t *effect);
extern bool     effect_is_known(int type);
extern bool     effect_learn(int type);
extern int      effect_value(effect_t *effect);
extern byte     effect_color(effect_t *effect);
extern cptr     do_effect(effect_t *effect_ptr, int mode, int boost);


/* do-spell.c */
extern int get_realm_idx(cptr name);
extern int beam_chance(void);
extern void cast_wonder(int dir);
extern int device_power(int pow);
extern int device_power_aux(int pow, int bonus);
extern int spell_power(int pow);
extern int spell_power_aux(int pow, int bonus);
extern int spell_cap(int cap);
extern int spell_cap_aux(int cap, int bonus);
extern cptr do_spell(int realm, int spell, int mode);
extern cptr info_damage(int dice, int sides, int base);
extern cptr info_duration(int base, int sides);
extern cptr info_range(int range);
extern cptr info_heal(int dice, int sides, int base);
extern cptr info_radius(int rad);
extern cptr info_power(int power);
extern cptr info_delay(int base, int sides);
extern cptr info_weight(int weight);
extern bool cast_summon_greater_demon(void);
extern bool cast_wrath_of_the_god(int dam, int rad);
bool trump_summoning(int num, bool pet, int y, int x, int lev, int type, u32b mode);

/* dungeon.c */
extern void leave_quest_check(void);
extern void extract_option_vars(void);
extern void determine_bounty_uniques(void);
extern void determine_today_mon(bool conv_old);
extern void notice_lite_change(object_type *o_ptr);
extern void play_game(bool new_game);
extern bool psychometry(void);
extern void leave_level(int level);
extern void enter_level(int level);
extern s32b turn_real(s32b hoge);
extern void prevent_turn_overflow(void);
extern void process_world_aux_movement(void);  /* yuk!  refactor the recall code instead */
extern void fame_on_failure(void);
extern void recharged_notice(object_type *o_ptr);
extern byte value_check_aux1(object_type *o_ptr); /* pseudo-id */

/* files.c */
extern cptr map_name(void);
extern void safe_setuid_drop(void);
extern void safe_setuid_grab(void);
extern s16b tokenize(char *buf, s16b num, char **tokens, int mode);
extern int z_string_split(char *buf, char **tokens, int max, cptr delim);
extern int parse_args(char *buf, char **name, char **args, int max);
extern void trim_tokens(char **tokens, int ct);
extern errr process_pref_file_command(char *buf);
extern cptr process_pref_file_expr(char **sp, char *fp);
extern errr process_pref_file(cptr name);
extern errr process_autopick_file(cptr name);
extern errr check_time_init(void);
extern errr check_load_init(void);
extern errr check_time(void);
extern errr check_load(void);
extern bool show_file(bool show_version, cptr name, cptr what, int line, int mode);
extern void do_cmd_help(void);
extern void process_player_name(bool sf);
extern bool py_get_name(void);
extern void do_cmd_suicide(void);
extern void do_cmd_save_game(int is_autosave);
extern void do_cmd_save_and_exit(void);
extern long total_points(void);
extern void close_game(void);
extern void exit_game_panic(void);
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);
extern errr get_rnd_line(cptr file_name, int entry, char *output);

extern void player_flags(u32b flgs[OF_ARRAY_SIZE]);
extern void tim_player_flags(u32b flgs[OF_ARRAY_SIZE]);
extern void tim_player_stats(s16b stats[MAX_STATS]);

extern int ct_kills(void);
extern int ct_kills_all(void);
extern int ct_uniques(void);
extern int ct_artifacts(void);

extern errr counts_write(int where, u32b count);
extern u32b counts_read(int where);
extern bool arg_lock_name; /*locks player name for server play --phantom*/

/* flavor.c */
extern void get_table_name_aux(char *out_string);
extern void get_table_name(char *out_string);
extern void get_table_sindarin_aux(char *out_string);
extern void get_table_sindarin(char *out_string);
extern void flavor_init(void);
extern char tval_to_attr_char(int tval);
extern char attr_to_attr_char(byte a);
extern char *object_desc_kosuu(char *t, object_type *o_ptr);
extern void object_desc(char *buf, object_type *o_ptr, u32b mode);

/* floors.c */
extern void init_saved_floors(bool force);
extern void clear_saved_floor_files(void);
extern saved_floor_type *get_sf_ptr(s16b floor_id);
extern s16b get_new_floor_id(void);
extern void prepare_change_floor_mode(u32b mode);
extern void precalc_cur_num_of_pet(void);
extern void leave_floor(void);
extern void change_floor(void);
extern void stair_creation(bool down_only);

/* generate.c */
extern bool place_quest_monsters(void);
extern void wipe_generate_cave_flags(void);
extern void clear_cave(void);
extern void generate_cave(void);

/* init1.c */
extern byte color_char_to_attr(char c);
extern s16b f_tag_to_index(cptr str);
extern void drop_here(object_type *j_ptr, int y, int x);
extern errr process_dungeon_file(cptr name, int ymin, int xmin, int ymax, int xmax);

/* init2.c */
extern void init_file_paths(const char *configpath, const char *libpath, const char *datapath);
extern void create_needed_dirs(void);
extern cptr err_str[PARSE_ERROR_MAX];
extern errr init_v_info(void);
extern errr init_buildings(void);
extern s16b f_tag_to_index_in_init(cptr str);
extern void init_angband(void);
extern void display_news(void);

/* load.c */
extern errr rd_savefile_new(void);
extern bool load_floor(saved_floor_type *sf_ptr, u32b mode);
extern void rd_item(savefile_ptr file, object_type *o_ptr);
extern void wr_item(savefile_ptr file, object_type *o_ptr); /* save.c */

/* melee1.c */
int check_hit(int power, int level, int stun, int m_idx);

/* melee2.c */
extern bool retaliation_hack;
extern int retaliation_count;
extern bool make_attack_normal(int m_idx);
extern void process_monsters(void);
extern int get_mproc_idx(int m_idx, int mproc_type);
extern void mproc_init(void);
extern bool set_monster_csleep(int m_idx, int v);
extern bool set_monster_fast(int m_idx, int v);
extern bool set_monster_slow(int m_idx, int v);
extern bool set_monster_stunned(int m_idx, int v);
extern bool set_monster_confused(int m_idx, int v);
extern bool set_monster_monfear(int m_idx, int v);
extern bool set_monster_invulner(int m_idx, int v, bool energy_need);
extern bool set_monster_paralyzed(int m_idx, int v);
extern void process_monsters_mtimed(int mtimed_idx);
extern void dispel_monster_status(int m_idx);
extern u32b get_curse(int power, object_type *o_ptr);
extern void curse_equipment(int chance, int heavy_chance);
extern void mon_take_hit_mon(int m_idx, int dam, bool *fear, cptr note, int who);
extern bool process_the_world(int num, int who, bool vs_player);
extern void monster_gain_exp(int m_idx, int s_idx);

/* monster1.c */
extern bool mon_save_p(int r_idx, int stat);
extern bool mon_save_aux(int r_idx, int power);
extern void roff_top(int r_idx);
extern bool mon_hook_dungeon(int r_idx);

extern void mon_lore_1(monster_type *m_ptr, u32b mask);
extern void mon_lore_2(monster_type *m_ptr, u32b mask);
extern void mon_lore_3(monster_type *m_ptr, u32b mask);
extern void mon_lore_4(monster_type *m_ptr, u32b mask);
extern void mon_lore_5(monster_type *m_ptr, u32b mask);
extern void mon_lore_6(monster_type *m_ptr, u32b mask);
extern void mon_lore_r(monster_type *m_ptr, u32b mask);
extern void mon_lore_move(monster_type *m_ptr);

extern void mon_lore_aux_1(monster_race *r_ptr, u32b mask);
extern void mon_lore_aux_2(monster_race *r_ptr, u32b mask);
extern void mon_lore_aux_3(monster_race *r_ptr, u32b mask);
extern void mon_lore_aux_4(monster_race *r_ptr, u32b mask);
extern void mon_lore_aux_5(monster_race *r_ptr, u32b mask);
extern void mon_lore_aux_6(monster_race *r_ptr, u32b mask);
extern void mon_lore_aux_r(monster_race *r_ptr, u32b mask);
extern void mon_lore_aux_spell(monster_race *r_ptr);

#define MON_BLOW_SILLY   0x01
#define MON_BLOW_OBVIOUS 0x02
#define MON_BLOW_DAMAGE  0x04
extern void mon_lore_blows(monster_type *m_ptr, int which, int options);
extern void mon_lore_aux_blows(monster_race *r_ptr, int which, int options);

extern monster_hook_type get_wilderness_monster_hook(int x, int y);
extern monster_hook_type get_monster_hook(void);
extern monster_hook_type get_monster_hook2(int y, int x); /* x, y is standard!!! */
extern void set_friendly(monster_type *m_ptr);
extern void set_pet(monster_type *m_ptr);
extern void set_hostile(monster_type *m_ptr);
extern void anger_monster(monster_type *m_ptr);
extern bool monster_can_cross_terrain(s16b feat, monster_race *r_ptr, u16b mode);
extern bool monster_can_enter(int y, int x, monster_race *r_ptr, u16b mode);
extern bool are_enemies(monster_type *m_ptr1, monster_type *m_ptr2);
extern bool monster_has_hostile_align(monster_type *m_ptr, int pa_good, int pa_evil, monster_race *r_ptr);
extern bool monster_living(monster_race *r_ptr);
extern bool monster_magical(monster_race *r_ptr);
extern bool no_questor_or_bounty_uniques(int r_idx);


/* monster2.c */
extern cptr horror_desc[MAX_SAN_HORROR];
extern cptr funny_desc[MAX_SAN_FUNNY];
extern cptr funny_comments[MAX_SAN_COMMENT];
extern void set_target(monster_type *m_ptr, int y, int x);
extern void reset_target(monster_type *m_ptr);
extern monster_race *real_r_ptr(monster_type *m_ptr);
extern int real_r_idx(monster_type *m_ptr);
extern void delete_monster_idx(int i);
extern void delete_monster(int y, int x);
extern void compact_monsters(int size);
extern void wipe_m_list(void);
extern bool mon_attack_mon(int m_idx, int t_idx);

extern bool mon_is_type(int r_idx, int type); /* Uses the various SUMMON_* constants */

extern s16b pack_info_pop(void);
extern void pack_info_push(int idx);
extern void pack_info_wipe(void);
extern void pack_on_slay_monster(int m_idx);
extern void pack_on_damage_monster(int m_idx);
extern pack_info_t *pack_info_ptr(int m_idx);
extern void pack_choose_ai(int m_idx);
extern bool mon_has_attack_spell(int m_idx);
extern bool mon_has_worthy_attack_spell(int m_idx);
extern bool mon_has_summon_spell(int m_idx);

extern monster_type *mon_get_parent(monster_type *m_ptr);
extern void mon_set_parent(monster_type *m_ptr, int pm_idx);
extern s16b m_pop(void);
extern errr get_mon_num_prep(monster_hook_type monster_hook, monster_hook_type monster_hook2);
extern s16b get_mon_num(int level);
extern void monster_desc(char *desc, monster_type *m_ptr, int mode);
extern int lore_do_probe(int r_idx);
extern void lore_treasure(int m_idx, int num_item, int num_gold);
extern void sanity_blast(monster_type *m_ptr, bool necro);
extern void update_mon(int m_idx, bool full);
extern void update_monsters(bool full);
extern bool place_monster_aux(int who, int y, int x, int r_idx, u32b mode);
extern int  place_monster_one(int who, int y, int x, int r_idx, int pack_idx, u32b mode);
extern bool place_monster(int y, int x, u32b mode);
extern bool alloc_horde(int y, int x);
extern bool alloc_guardian(bool def_val);
extern bool alloc_monster(int dis, u32b mode);
extern bool summon_specific(int who, int y1, int x1, int lev, int type, u32b mode);
extern bool summon_named_creature (int who, int oy, int ox, int r_idx, u32b mode);
extern bool multiply_monster(int m_idx, bool clone, u32b mode);
extern void update_smart_learn(int m_idx, int what);
extern void choose_new_monster(int m_idx, bool born, int r_idx);
extern byte get_mspeed(monster_race *r_ptr);
extern bool player_place(int y, int x);
extern void monster_drop_carried_objects(monster_type *m_ptr);

/* mon_display.c */
extern void mon_display(monster_race *r_ptr);
extern void mon_display_rect(monster_race *r_ptr, rect_t display);
extern void mon_display_doc(monster_race *r_ptr, doc_ptr doc);

/* obj_display.c */
extern void obj_display(object_type *o_ptr);
extern void obj_display_rect(object_type *o_ptr, rect_t display);
extern void obj_display_doc(object_type *o_ptr, doc_ptr doc);
extern void obj_display_smith(object_type *o_ptr, doc_ptr doc);
extern void device_display_doc(object_type *o_ptr, doc_ptr doc);
extern void ego_display(ego_type *e_ptr);
extern void ego_display_rect(ego_type *e_ptr, rect_t display);
extern void ego_display_doc(ego_type *e_ptr, doc_ptr doc);

/* py_display.c */
extern void py_display(void);
extern void py_display_birth(void);
extern void py_display_spells(doc_ptr doc, spell_info *table, int ct);
extern void py_display_powers(doc_ptr doc, spell_info *table, int ct);
extern void py_display_character_sheet(doc_ptr doc);
extern void py_display_dungeons(doc_ptr doc);

/* object1.c */
extern s16b m_bonus(int max, int level);

extern void reset_visuals(void);
extern void obj_flags(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE]);
extern void weapon_flags(int hand, u32b flgs[OF_ARRAY_SIZE]);
extern void weapon_flags_known(int hand, u32b flgs[OF_ARRAY_SIZE]);
extern void missile_flags(object_type *arrow, u32b flgs[OF_ARRAY_SIZE]);
extern void missile_flags_known(object_type *arrow, u32b flgs[OF_ARRAY_SIZE]);
extern char index_to_label(int i);
extern s16b label_to_inven(int c);
extern s16b label_to_equip(int c);
extern cptr describe_use(int i);
extern bool check_book_realm(const byte book_tval, const byte book_sval);
extern bool item_tester_okay(object_type *o_ptr);
extern void display_inven(void);
extern void display_equip(void);
extern int show_inven(int target_item, int mode);
extern int show_equip(int target_item, int mode);
extern void toggle_inven_equip(void);
extern bool can_get_item(void);
extern bool get_item(int *cp, cptr pmt, cptr str, int mode);

/* Object Lore */
extern void obj_flags_known(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE]);
extern void obj_flags_unknown(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE]);
extern bool obj_is_identified(object_type *o_ptr);
extern bool obj_is_identified_fully(object_type *o_ptr);
extern void obj_identify(object_type *o_ptr);
extern void obj_identify_fully(object_type *o_ptr);

/* obj_learn_*() methods return TRUE if the flag is present and unknown (hence learned)
   and FALSE otherwise. So FALSE means either the flag is not applicable, or that
   the user already knew about it. Generally, TRUE indicates the need to display
   a message to the player. */
extern bool obj_learn_flag(object_type *o_ptr, int which);
extern void obj_learn_activation(object_type *o_ptr);
extern bool obj_learn_curse(object_type *o_ptr, int flag);

/* Learn a weapon slay *and* alert the user. */
extern void obj_learn_slay(object_type *o_ptr, int which, cptr msg);

/* Equipping an object learns many things, mostly pval related. Alert the user. */
extern void obj_learn_equipped(object_type *o_ptr);

extern bool ego_has_lore(ego_type *e_ptr);
extern bool art_has_lore(artifact_type *a_ptr);
extern bool obj_has_lore(object_type *o_ptr);

/* ego.c */
extern void ego_create_ring(object_type *o_ptr, int level, int power, int mode);
extern void ego_create_amulet(object_type *o_ptr, int level, int power, int mode);
extern bool obj_create_device(object_type *o_ptr, int level, int power, int mode);
extern void obj_create_weapon(object_type *o_ptr, int level, int power, int mode);
extern void obj_create_armor(object_type *o_ptr, int level, int power, int mode);
extern void obj_create_lite(object_type *o_ptr, int level, int power, int mode);
extern int  ego_choose_type(int type, int level);
extern void ego_weapon_adjust_weight(object_type *o_ptr);
extern void ego_brand_weapon(object_type *o_ptr, int which);
extern void ego_finalize(object_type *o_ptr, int level, int power, int mode);

/* object2.c */
extern bool add_esp_strong(object_type *o_ptr);
extern void add_esp_weak(object_type *o_ptr, bool extra);
extern void excise_object_idx(int o_idx);
extern void delete_object_idx(int o_idx);
extern void delete_object(int y, int x);
extern void compact_objects(int size);
extern void wipe_o_list(void);
extern s16b o_pop(void);
extern s16b get_obj_num(int level);
extern errr get_obj_num_prep(void);
extern bool object_is_aware(object_type *o_ptr);
extern void object_aware(object_type *o_ptr);
extern void object_tried(object_type *o_ptr);
extern bool object_is_tried(object_type *o_ptr);
extern s32b obj_value(object_type *o_ptr);
extern s32b obj_value_real(object_type *o_ptr);
extern s32b obj_android_exp(object_type *o_ptr);
extern bool can_player_destroy_object(object_type *o_ptr);
extern void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
extern void reduce_charges(object_type *o_ptr, int amt);
extern int object_similar_part(object_type *o_ptr, object_type *j_ptr);
extern bool object_similar(object_type *o_ptr, object_type *j_ptr);
extern void object_absorb(object_type *o_ptr, object_type *j_ptr);
extern s16b lookup_kind(int tval, int sval);
extern void object_wipe(object_type *o_ptr);
extern void object_prep(object_type *o_ptr, int k_idx);
extern void object_copy(object_type *o_ptr, object_type *j_ptr);
extern bool apply_magic(object_type *o_ptr, int lev, u32b mode);
extern int  apply_magic_ego;
extern void choose_obj_kind(int mode); /* Hack for BM to use new object tval frequencies */
extern bool make_object(object_type *j_ptr, u32b mode);
extern bool kind_is_good(int k_idx);
extern bool kind_is_great(int k_idx);
extern bool kind_is_device(int k_idx);
extern bool kind_is_jewelry(int k_idx);
extern bool kind_is_book(int k_idx);
extern bool kind_is_body_armor(int k_idx);
extern bool kind_is_helm(int k_idx);
extern bool kind_is_wand_rod_staff(int k_idx);
extern bool kind_is_other_armor(int k_idx);
extern bool kind_is_armor(int k_idx);
extern bool kind_is_weapon(int k_idx);
extern bool kind_is_bow_ammo(int k_idx);
extern bool kind_is_misc(int k_idx);
extern void place_object(int y, int x, u32b mode);
extern bool make_gold(object_type *j_ptr, bool do_boost);
extern void place_gold(int y, int x);
extern s16b drop_near(object_type *o_ptr, int chance, int y, int x);
extern void acquirement(int y1, int x1, int num, int greatness, bool known);
extern void init_normal_traps(void);
extern s16b choose_random_trap(void);
extern void disclose_grid(int y, int x);
extern void place_trap(int y, int x);
extern void inven_item_charges(int item);
extern void inven_item_describe(int item);
extern void inven_item_increase(int item, int num);
extern void inven_item_optimize(int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern bool inven_carry_okay(object_type *o_ptr);
extern bool object_sort_comp(object_type *o_ptr, s32b o_value, object_type *j_ptr);
extern s16b inven_carry(object_type *o_ptr);
extern s16b inven_takeoff(int item, int amt);
extern void inven_drop(int item, int amt);
extern void combine_pack(void);
extern void reorder_pack(void);
extern void display_koff(int k_idx);
extern object_type *choose_warning_item(void);
extern bool process_warning(int xx, int yy);
extern void stats_on_purchase(object_type *o_ptr);
extern void stats_on_sell(object_type *o_ptr);
extern void stats_on_notice(object_type *o_ptr, int num);
extern void stats_on_combine(object_type *dest, object_type *src);
extern void stats_on_use(object_type *o_ptr, int num);
extern void stats_on_p_destroy(object_type *o_ptr, int num);
extern void stats_on_m_destroy(object_type *o_ptr, int num);
extern void stats_on_pickup(object_type *o_ptr);
extern void stats_on_equip(object_type *o_ptr);
extern void stats_on_identify(object_type *o_ptr);
extern void stats_on_load(savefile_ptr file);
extern void stats_on_save(savefile_ptr file);
extern void stats_on_gold_find(int au);
extern void stats_on_gold_selling(int au);
extern void stats_on_gold_buying(int au);
extern void stats_on_gold_services(int au);
extern void stats_on_gold_winnings(int au);
extern void stats_on_gold_stolen(int au);
extern void stats_reset(void);
extern counts_t stats_rand_art_counts;
extern gold_counts_t stats_gold_counts;

/* object3.c */
typedef void (*debug_hook)(cptr msg);
extern debug_hook cost_calc_hook;
enum { COST_REAL = 0x01 };
extern s32b weapon_cost(object_type *o_ptr, int options);
extern s32b ammo_cost(object_type *o_ptr, int options);
extern s32b bow_cost(object_type *o_ptr, int options);
extern s32b armor_cost(object_type *o_ptr, int options);
extern s32b jewelry_cost(object_type *o_ptr, int options);
extern s32b lite_cost(object_type *o_ptr, int options);
extern s32b new_object_cost(object_type *o_ptr, int options);

/* racial.c */
extern bool can_do_cmd_cast(void);
extern void stop_mouth(void);

/* save.c */
extern bool save_player(void);
extern bool load_player(void);
extern void remove_loc(void);
extern bool save_floor(saved_floor_type *sf_ptr, u32b mode);

/* spells1.c */
extern bool allow_ticked_off(monster_race *r_ptr);
extern bool in_disintegration_range(int y1, int x1, int y2, int x2);
extern void breath_shape(u16b *path_g, int dist, int *pgrids, byte *gx, byte *gy, byte *gm, int *pgm_rad, int rad, int y1, int x1, int y2, int x2, int typ);
extern int take_hit(int damage_type, int damage, cptr kb_str, int monspell);
extern u16b bolt_pict(int y, int x, int ny, int nx, int typ);
extern sint project_path(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg);
extern int dist_to_line(int y, int x, int y1, int x1, int y2, int x2);
extern bool project(int who, int rad, int y, int x, int dam, int typ, int flg, int monspell);
extern bool project_m(int who, int r, int y, int x, int dam, int typ, int flg, bool see_s_msg);
extern int project_length;
extern bool binding_field(int dam);

/* spells2.c */
extern void message_pain(int m_idx, int dam);
extern void self_knowledge(void);
extern bool detect_traps(int range, bool known);
extern bool detect_doors(int range);
extern bool detect_stairs(int range);
extern bool detect_treasure(int range);
extern bool detect_objects_gold(int range);
extern bool detect_objects_normal(int range);
extern bool detect_objects_magic(int range);
extern bool detect_monsters_normal(int range);
extern bool detect_monsters_invis(int range);
extern bool detect_monsters_evil(int range);
extern bool detect_monsters_xxx(int range, u32b match_flag);
extern bool detect_monsters_string(int range, cptr);
extern bool detect_monsters_nonliving(int range);
extern bool detect_monsters_living(int range, cptr msg);
extern bool detect_monsters_mind(int range);
extern bool detect_monsters_magical(int range);
extern bool detect_all(int range);
extern bool wall_stone(void);
extern bool speed_monsters(void);
extern bool slow_monsters(int power);
extern bool sleep_monsters(int power);
extern void aggravate_monsters(int who);
extern bool genocide_aux(int m_idx, int power, bool player_cast, int dam_side, cptr spell_name);
extern bool symbol_genocide(int power, bool player_cast);
extern bool mass_genocide(int power, bool player_cast);
extern bool mass_genocide_undead(int power, bool player_cast);
extern bool probing(void);
extern bool banish_evil(int dist);
extern bool dispel_evil(int dam);
extern bool dispel_good(int dam);
extern bool dispel_undead(int dam);
extern bool dispel_monsters(int dam);
extern bool dispel_living(int dam);
extern bool dispel_demons(int dam);
extern bool turn_undead(void);
extern bool destroy_area(int y1, int x1, int r, int power);
extern bool earthquake_aux(int cy, int cx, int r, int m_idx);
extern bool earthquake(int cy, int cx, int r);
extern void lite_room(int y1, int x1);
extern void unlite_room(int y1, int x1);
extern bool lite_area(int dam, int rad);
extern bool unlite_area(int dam, int rad);
extern bool fire_ball(int typ, int dir, int dam, int rad);
extern bool fire_ball_aux(int typ, int dir, int dam, int rad, int xtra_flgs);
extern bool fire_rocket(int typ, int dir, int dam, int rad);
extern bool fire_ball_hide(int typ, int dir, int dam, int rad);
extern bool fire_meteor(int who, int typ, int x, int y, int dam, int rad);
extern bool fire_bolt(int typ, int dir, int dam);
extern bool fire_blast(int typ, int dir, int dd, int ds, int num, int dev);
extern void call_chaos(int pct);
extern bool fire_beam(int typ, int dir, int dam);
extern bool fire_bolt_or_beam(int prob, int typ, int dir, int dam);
extern bool lite_line(int dir);
extern bool drain_life(int dir, int dam);
extern bool wall_to_mud(int dir);
extern bool destroy_door(int dir);
extern bool disarm_trap(int dir);
extern bool wizard_lock(int dir);
extern bool heal_monster(int dir, int dam);
extern bool speed_monster(int dir);
extern bool slow_monster(int dir);
extern bool sleep_monster(int dir, int power);
extern bool stasis_monster(int dir);    /* Like sleep, affects undead as well */
extern bool stasis_evil(int dir);    /* Like sleep, affects undead as well */
extern bool confuse_monster(int dir, int plev);
extern bool stun_monster(int dir, int plev);
extern bool fear_monster(int dir, int plev);
extern bool poly_monster(int dir);
extern bool clone_monster(int dir);
extern bool teleport_monster(int dir);
extern bool door_creation(void);
extern bool trap_creation(int y, int x);
extern bool tree_creation(void);
extern bool glyph_creation(void);
extern bool destroy_doors_touch(void);
extern bool animate_dead(int who, int y, int x);
extern bool sleep_monsters_touch(void);
extern bool activate_ty_curse(bool stop_ty, int *count);
extern int activate_hi_summon(int y, int x, bool can_pet);
extern int summon_cyber(int who, int y, int x);
extern void wall_breaker(void);
extern bool confuse_monsters(int dam);
extern bool charm_monsters(int dam);
extern bool charm_animals(int dam);
extern bool stun_monsters(int dam);
extern bool stasis_monsters(int dam);
extern bool banish_monsters(int dist);
extern bool turn_monsters(int dam);
extern bool turn_evil(int dam);
extern bool deathray_monsters(void);
extern bool charm_monster(int dir, int plev);
extern bool control_one_undead(int dir, int plev);
extern bool control_one_demon(int dir, int plev);
extern bool charm_animal(int dir, int plev);
extern bool charm_living(int dir, int plev);
extern bool mindblast_monsters(int dam);
extern void report_magics(void);
extern bool teleport_swap(int dir);
extern bool project_hook(int typ, int dir, int dam, int flg);
extern bool project_hack(int typ, int dam);
extern bool eat_magic(int power);
extern void discharge_minion(void);
extern bool kawarimi(bool success);
extern bool rush_attack(int rng, bool *mdeath);


/* spells3.c */
extern bool dimension_door_aux(int x, int y, int rng);
extern bool teleport_away(int m_idx, int dis, u32b mode);
extern void teleport_monster_to(int m_idx, int ty, int tx, int power, u32b mode);
extern bool cave_player_teleportable_bold(int y, int x, u32b mode);
extern bool teleport_player_aux(int dis, u32b mode);
extern void teleport_player(int dis, u32b mode);
extern void teleport_player_away(int m_idx, int dis);
extern void teleport_player_to(int ny, int nx, u32b mode);
extern void teleport_away_followable(int m_idx);
extern void teleport_level(int m_idx);
extern int choose_dungeon(cptr note, int y, int x);
extern bool recall_player(int turns);
extern bool word_of_recall(void);
extern bool reset_recall(void);
extern bool apply_disenchant(int mode);
extern void mutate_player(void);
extern void apply_nexus(monster_type *m_ptr);
extern void phlogiston(void);
extern bool brand_weapon(int brand_type);
extern bool brand_weapon_aux(int item);
extern bool brand_armour_aux(int item);
extern bool brand_weapon_slaying(int brand_flag, int res_flag);
extern void call_the_(void);
extern void fetch(int dir, int wgt, bool require_los);
extern void alter_reality(void);
extern bool warding_glyph(void);
extern bool explosive_rune(void);
extern bool set_trap(int y, int x, int feature);
extern void identify_pack(void);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool alchemy(void);
extern bool enchant(object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac);
extern bool item_tester_hook_nameless_weapon_armour(object_type *o_ptr);
extern bool artifact_scroll(void);
extern bool ident_spell(object_p p);
extern bool identify_fully(object_p p);
extern bool mundane_spell(bool only_equip);
extern bool identify_item(object_type *o_ptr);
extern bool recharge_from_player(int power);
extern bool recharge_from_device(int power);
extern bool bless_weapon(void);
extern bool polish_shield(void);
extern bool potion_smash_effect(int who, int y, int x, int k_idx);
extern s16b experience_of_spell(int spell, int realm);
extern int mod_need_mana(int need_mana, int spell, int realm);
extern int mod_spell_chance_1(int chance, int realm);
extern int mod_spell_chance_2(int chance, int realm);
extern s16b spell_chance(int spell,int realm);
extern bool spell_okay(int spell, bool learned, bool study_pray, int realm);
extern void print_spells(int target_spell, byte *spells, int num, rect_t display, int use_realm);
extern bool hates_acid(object_type *o_ptr);
extern bool hates_elec(object_type *o_ptr);
extern bool hates_fire(object_type *o_ptr);
extern bool hates_cold(object_type *o_ptr);
extern int set_acid_destroy(object_type *o_ptr);
extern int set_elec_destroy(object_type *o_ptr);
extern int set_fire_destroy(object_type *o_ptr);
extern int set_cold_destroy(object_type *o_ptr);
extern int inven_damage(inven_func typ, int perc, int which);
extern int acid_dam(int dam, cptr kb_str, int monspell);
extern int elec_dam(int dam, cptr kb_str, int monspell);
extern int fire_dam(int dam, cptr kb_str, int monspell);
extern int cold_dam(int dam, cptr kb_str, int monspell);
extern bool rustproof(void);
extern bool curse_armor(int slot);
extern bool curse_weapon(bool force, int slot);
extern void blast_object(object_type *o_ptr);
extern bool polymorph_monster(int y, int x);
extern bool dimension_door(int rng);
extern bool summon_kin_player(int level, int y, int x, u32b mode);

/* store.c */
extern bool store_hack;
extern bool combine_and_reorder_home(int store_num);
extern void do_cmd_store(void);
extern void store_shuffle(int which);
enum { STORE_MAINT_CULL = 1, STORE_MAINT_NORMAL };
extern void store_maint(int town_num, int store_num, int options);
extern void store_init(int town_num, int store_num);
extern int  store_calc_price_factor(int greed);
extern int  store_calc_sell_price(int price, int factor);
extern int  store_calc_purchase_price(int price, int factor);
extern void move_to_black_market(object_type * o_ptr);
extern void mass_produce(object_type *o_ptr);

/* bldg.c */
extern int get_bldg_member_code(cptr name);
extern bool get_nightmare(int r_idx);
extern void have_nightmare(int r_idx);
extern void battle_monsters(void);
extern void do_cmd_bldg(void);
extern void do_cmd_quest(void);
extern void quest_discovery(int q_idx);
extern int quest_number(int level);
extern int random_quest_number(int level);
extern bool tele_town(void);
extern int hit_chance(int hand, int to_h, int ac);

/* combat.c */
extern int bow_energy(int sval);
extern int bow_range(object_type *o_ptr);
extern int bow_mult(object_type *o_ptr);
extern void display_weapon_info(doc_ptr doc, int hand);
extern int display_weapon_mode;
extern void display_innate_attack_info(doc_ptr doc, int which);
extern void display_shooter_info(doc_ptr doc);
extern int calculate_base_blows(int hand, int str_idx, int dex_idx);

/* util.c */
extern errr path_parse(char *buf, int max, cptr file);
size_t path_build(char *buf, size_t len, const char *base, const char *leaf);
extern FILE *my_fopen(cptr file, cptr mode);
extern FILE *my_fopen_temp(char *buf, int max);
extern errr my_fgets(FILE *fff, char *buf, huge n);
extern errr my_fputs(FILE *fff, cptr buf, huge n);
extern errr my_fclose(FILE *fff);
extern errr fd_kill(cptr file);
extern errr fd_move(cptr file, cptr what);
extern errr fd_copy(cptr file, cptr what);
extern int fd_make(cptr file, int mode);
extern int fd_open(cptr file, int flags);
extern errr fd_lock(int fd, int what);
extern errr fd_seek(int fd, huge n);
extern errr fd_chop(int fd, huge n);
extern errr fd_read(int fd, char *buf, huge n);
extern errr fd_write(int fd, cptr buf, huge n);
extern errr fd_close(int fd);
extern void flush(void);
extern void bell(void);
extern void sound(int num);
extern void move_cursor(int row, int col);
extern void text_to_ascii(char *buf, cptr str);
extern void ascii_to_text(char *buf, cptr str);
extern errr macro_add(cptr pat, cptr act);
extern sint macro_find_exact(cptr pat);
extern char inkey(void);
extern cptr quark_str(s16b num);
extern void quark_init(void);
extern s16b quark_add(cptr str);

extern void screen_save(void);
extern void screen_save_aux(void);
extern void screen_load(void);
extern void screen_load_aux(void);
extern bool screen_is_saved(void);
extern void c_put_str(byte attr, cptr str, int row, int col);
extern void put_str(cptr str, int row, int col);
extern void c_prt(byte attr, cptr str, int row, int col);
extern void prt(cptr str, int row, int col);
extern void c_roff(byte attr, cptr str);
extern void roff(cptr str);
extern void clear_from(int row);
extern bool askfor_aux(char *buf, int len, bool numpad_cursor);
extern bool askfor(char *buf, int len);
extern bool get_string(cptr prompt, char *buf, int len);
extern bool get_check(cptr prompt);
extern bool get_check_strict(cptr prompt, int mode);
extern bool get_com(cptr prompt, char *command, bool z_escape);
extern s16b get_quantity(cptr prompt, int max);
extern void pause_line(int row);
extern void pause_line_aux(cptr prompt, int row, int col);
extern void request_command(int shopping);
extern bool is_a_vowel(int ch);
extern int get_keymap_dir(char ch);
extern errr type_string(cptr str, uint len);
extern void roff_to_buf(cptr str, int wlen, char *tbuf, size_t bufsize);

#ifdef SORT_R_INFO
extern void tag_sort(tag_type elements[], int number);
#endif /* SORT_R_INFO */

#ifdef SUPPORT_GAMMA
extern byte gamma_table[256];
extern void build_gamma_table(int gamma);
#endif /* SUPPORT_GAMMA */

extern size_t my_strcpy(char *buf, const char *src, size_t bufsize);
extern size_t my_strcat(char *buf, const char *src, size_t bufsize);
extern char *my_strstr(const char *haystack, const char *needle);
extern char *my_strchr(const char *ptr, char ch);
extern void str_tolower(char *str);
extern int inkey_special(bool numpad_cursor);


/* xtra1.c */
extern void cnv_stat(int val, char *out_val);
extern s16b modify_stat_value(int value, int amount);
extern bool is_daytime(void);
extern void extract_day_hour_min(int *day, int *hour, int *min);
extern void extract_day_hour_min_imp(int turn, int *day, int *hour, int *min);
extern void prt_time(void);
extern u32b weight_limit(void);
extern void calc_bonuses(void);
extern void calc_innate_blows(innate_attack_ptr a, int max);
extern void notice_stuff(void);
extern void update_stuff(void);
extern void redraw_stuff(void);
extern void window_stuff(void);
extern void handle_stuff(void);
extern bool heavy_armor(void);
extern int  py_prorata_level(int amt);
extern int  py_prorata_level_aux(int amt, int w1, int w2, int w3);
extern int  big_num_round(int num, int sig_figs);
extern void big_num_display(int num, char *buf);
extern void check_mon_health_redraw(int m_idx);

/* effects.c */
extern void set_action(int typ);
extern void reset_tim_flags(void);
extern void dispel_player(void);
extern bool disenchant_player(void);
extern bool set_mimic(int v, int p, bool do_dec);
extern bool set_blind(int v, bool do_dec);
extern bool set_confused(int v, bool do_dec);
extern bool set_poisoned(int v, bool do_dec);
extern bool set_paralyzed(int v, bool do_dec);
extern bool set_image(int v, bool do_dec);
extern bool set_fast(int v, bool do_dec);
extern bool set_slow(int v, bool do_dec);
extern bool set_shield(int v, bool do_dec);
extern bool set_tsubureru(int v, bool do_dec);
extern bool set_magicdef(int v, bool do_dec);
extern bool set_blessed(int v, bool do_dec);
extern bool set_hero(int v, bool do_dec);
extern bool set_shero(int v, bool do_dec);
extern bool set_protevil(int v, bool do_dec);
extern bool set_invuln(int v, bool do_dec);
extern bool set_tim_invis(int v, bool do_dec);
extern bool set_tim_infra(int v, bool do_dec);
extern bool set_tim_regen(int v, bool do_dec);
extern bool set_tim_stealth(int v, bool do_dec);
extern bool set_lightspeed(int v, bool do_dec);
extern bool set_tim_levitation(int v, bool do_dec);
extern bool set_tim_sh_touki(int v, bool do_dec);
extern bool set_tim_sh_fire(int v, bool do_dec);
extern bool set_tim_sh_elements(int v, bool do_dec);
extern bool set_tim_sh_shards(int v, bool do_dec);
extern bool set_tim_sh_domination(int v, bool do_dec);
extern bool set_tim_weaponmastery(int v, bool do_dec);
extern bool set_tim_sh_holy(int v, bool do_dec);
extern bool set_tim_eyeeye(int v, bool do_dec);
extern bool set_tim_spurt(int v, bool do_dec);
extern bool set_tim_speed_essentia(int v, bool do_dec);
extern bool set_tim_shrike(int v, bool do_dec);
extern bool set_tim_blood_rite(int v, bool do_dec);
extern bool set_tim_blood_shield(int v, bool do_dec);
extern bool set_tim_blood_seek(int v, bool do_dec);
extern bool set_tim_blood_sight(int v, bool do_dec);
extern bool set_tim_blood_feast(int v, bool do_dec);
extern bool set_tim_blood_revenge(int v, bool do_dec);
extern bool set_tim_genji(int v, bool do_dec);
extern bool set_tim_force(int v, bool do_dec);
extern bool set_tim_building_up(int v, bool do_dec);
extern bool set_tim_vicious_strike(int v, bool do_dec);
extern bool set_tim_enlarge_weapon(int v, bool do_dec);
extern bool set_tim_superstealth(int v, bool do_dec);
extern bool set_tim_no_spells(int v, bool do_dec);
extern bool set_tim_no_device(int v, bool do_dec);
extern bool set_resist_magic(int v, bool do_dec);
extern bool set_tim_reflect(int v, bool do_dec);
extern bool set_tim_no_tele(int v, bool do_dec);

extern bool set_tim_spell_reaction(int v, bool do_dec);
extern bool set_tim_resist_curses(int v, bool do_dec);
extern bool set_tim_armor_of_fury(int v, bool do_dec);
extern bool set_tim_spell_turning(int v, bool do_dec);

extern bool set_tim_dark_stalker(int v, bool do_dec);
extern bool set_tim_nimble_dodge(int v, bool do_dec);
extern bool set_tim_stealthy_snipe(int v, bool do_dec);

extern bool set_tim_killing_spree(int v, bool do_dec);
extern bool set_tim_slay_sentient(int v, bool do_dec);

extern bool set_tim_sustain_str(int v, bool do_dec);
extern bool set_tim_sustain_int(int v, bool do_dec);
extern bool set_tim_sustain_wis(int v, bool do_dec);
extern bool set_tim_sustain_dex(int v, bool do_dec);
extern bool set_tim_sustain_con(int v, bool do_dec);
extern bool set_tim_sustain_chr(int v, bool do_dec);
extern bool set_tim_hold_life(int v, bool do_dec);
extern bool set_tim_transcendence(int v, bool do_dec);
extern bool set_tim_quick_walk(int v, bool do_dec);
extern bool set_tim_inven_prot(int v, bool do_dec);
extern bool set_tim_device_power(int v, bool do_dec);
extern bool set_tim_sh_time(int v, bool do_dec);
extern bool set_tim_foresight(int v, bool do_dec);

extern bool set_multishadow(int v, bool do_dec);
extern bool set_dustrobe(int v, bool do_dec);
extern bool set_kabenuke(int v, bool do_dec);
extern bool set_tsuyoshi(int v, bool do_dec);
extern bool set_ele_attack(u32b attack_type, int v);
extern bool set_ele_immune(u32b immune_type, int v);
extern bool set_oppose_acid(int v, bool do_dec);
extern bool set_oppose_elec(int v, bool do_dec);
extern bool set_oppose_fire(int v, bool do_dec);
extern bool set_oppose_cold(int v, bool do_dec);
extern bool set_oppose_pois(int v, bool do_dec);
extern bool set_stun(int v, bool do_dec);
extern bool set_cut(int v, bool do_dec);
typedef struct { int level; int dam; cptr desc; byte attr; } cut_info_t;
extern cut_info_t cut_info(int v);
extern bool set_food(int v);
extern bool inc_stat(int stat);
extern bool dec_stat(int stat, int amount, int permanent);
extern bool res_stat(int stat);
extern bool hp_player(int num);
extern bool hp_player_aux(int num);
extern bool sp_player(int num);
extern bool do_dec_stat(int stat);
extern bool do_res_stat(int stat);
extern bool do_inc_stat(int stat);
extern bool restore_level(void);
extern bool lose_all_info(void);
extern void gain_exp_64(s32b amount, u32b amount_frac);
extern void gain_exp(s32b amount);
extern void lose_exp(s32b amount);
extern bool drain_exp(s32b drain, s32b slip, int hold_life_prob);
extern void do_poly_self(void);
extern bool set_ultimate_res(int v, bool do_dec);
extern bool set_tim_res_nether(int v, bool do_dec);
extern bool set_tim_res_time(int v, bool do_dec);
extern bool set_tim_res_disenchantment(int v, bool do_dec);
extern bool choose_ele_attack(void);
extern bool choose_ele_immune(int turn);
extern bool set_wraith_form(int v, bool do_dec);
extern bool set_tim_esp(int v, bool do_dec);
extern bool set_tim_esp_magical(int v, bool do_dec);
extern bool set_superstealth(bool set);
extern bool set_sanctuary(bool set);

/* xtra2.c */
extern void check_experience(void);
extern int exp_requirement(int level);
extern void gain_chosen_stat(void);
extern void check_quest_completion(monster_type *m_ptr);
extern cptr extract_note_dies(monster_race *r_ptr);
extern void monster_death(int m_idx, bool drop_item);
extern bool get_monster_drop(int m_idx, object_type *o_ptr);
extern byte get_monster_drop_ct(monster_type *m_ptr);
extern bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note);
extern void mon_check_kill_unique(int m_idx);
extern void resize_map(void);
extern void redraw_window(void);

/* Display
   Various regions of the terminal are reserved for different things.
   For example, msgs are drawn near the top, the map is (now) leftmost, a
   status bar is on the bottom, and quick character info is (now) on the
   right. You can query the placement with: */
extern rect_t ui_map_rect(void);
extern rect_t ui_menu_rect(void);
extern rect_t ui_status_bar_rect(void);
extern rect_t ui_char_info_rect(void);
/* cf msg_line_rect() in message.h and note that the message "line" is
   really a drop down box of sorts. It may drop on top of whatever is beneath
   it (currently the map region). */

/* Previously, the map drawing/scrolling code was a confusing mess, so I
   cleaned things up. Here is how it works:
   [1] ui_map_rect() is the region of the terminal for display. Points
       inside this region are "ui points". These are the points you use
       for Term_putch, etc.
   [2] But, we display tiles from the cave. This is the current map. Think
       of the map_rect as a tiny window unto a much larger map that is the
       current dungeon level. Traditionally, this is called a "viewport",
       not a "panel".
   [3] The position of the "viewport" into the dungeon (i.e., the cave array)
       is given by the viewport_origin: */
extern point_t viewport_origin;

/* Now, you can translate points back and forth between the "ui" and the
   "cave". This is nothing more than computing displacement vectors and
   adding them to a new origin for the target coordinate system. Easy peasy: */
extern point_t ui_pt_to_cave_pt(point_t pt);
extern point_t ui_xy_to_cave_pt(int x, int y);
extern point_t cave_pt_to_ui_pt(point_t pt);
extern point_t cave_xy_to_ui_pt(int x, int y);

/* And you can query whether or not a "ui"/"cave" point is currently visible:*/
extern bool cave_pt_is_visible(point_t pt);
extern bool cave_xy_is_visible(int x, int y);
extern bool ui_pt_is_visible(point_t pt);
extern bool ui_xy_is_visible(int x, int y);

/* As well as make sure the player is currently visible, or slide the viewport
   around to display other areas of the map: */
#define VIEWPORT_FORCE_CENTER 0x01
extern void viewport_verify(void);
extern void viewport_verify_aux(u32b options);
extern bool viewport_scroll(int dy, int dx);

/* If you like, you should be able to alter the result of ui_map_rect() and
   things should just work. */


extern cptr mon_health_desc(monster_type *m_ptr);
extern cptr mon_allegiance_desc(monster_type *m_ptr);
extern void ang_sort_aux(vptr u, vptr v, int p, int q);
extern void ang_sort(vptr u, vptr v, int n);
extern bool target_able(int m_idx);
extern bool target_okay(void);
extern bool target_set(int mode);
extern bool get_aim_dir(int *dp);
extern bool get_hack_dir(int *dp);
extern bool get_rep_dir(int *dp, bool under);
extern bool get_rep_dir2(int *dp);
extern bool tgt_pt (int *x, int *y, int rng);
extern void do_poly_wounds(void);
extern void change_race(int new_race, cptr effect_msg);
extern int mon_damage_mod(monster_type *m_ptr, int dam, bool is_psy_spear);
extern int mon_damage_mod_mon(monster_type *m_ptr, int dam, bool is_psy_spear);
extern s16b gain_energy(void);
extern cptr your_alignment(void);
extern int weapon_exp_level(int weapon_exp);
extern int riding_exp_level(int riding_exp);
extern int spell_exp_level(int spell_exp);

/* mspells1.c */
extern bool clean_shot(int y1, int x1, int y2, int x2, bool friend);
extern bool summon_possible(int y1, int x1);
extern bool raise_possible(monster_type *m_ptr);
extern bool dispel_check(int m_idx);
extern bool spell_is_inate(u16b spell);
extern bool make_attack_spell(int m_idx, bool ticked_off);

/* mspells2.c */
extern void get_project_point(int sy, int sx, int *ty, int *tx, int flg);
#define DRAGONRIDER_HACK 0x01 /* sorry about this one, but damn, mon_spell_mon needs refactoring! I don't have the patience right now ... */
extern bool mon_spell_mon(int m_idx, int options);

/* artifact.c */
extern bool immunity_hack;
extern void one_sustain(object_type *o_ptr);
extern void one_high_resistance(object_type *o_ptr);
extern bool one_high_vulnerability(object_type *o_ptr);
extern void one_undead_resistance(object_type *o_ptr);
extern void one_demon_resistance(object_type *o_ptr);
extern void one_holy_resistance(object_type *o_ptr);
extern void one_lordly_high_resistance(object_type *o_ptr);
extern void one_ele_slay(object_type *o_ptr);
extern void one_ele_resistance(object_type *o_ptr);
extern bool one_ele_vulnerability(object_type *o_ptr);
extern void one_dragon_ele_resistance(object_type *o_ptr);
extern bool one_dragon_ele_vulnerability(object_type *o_ptr);
extern void one_low_esp(object_type *o_ptr);
extern void one_resistance(object_type *o_ptr);
extern bool one_vulnerability(object_type *o_ptr);
extern bool one_stat_biff(object_type *o_ptr);
extern bool one_biff(object_type *o_ptr);
extern void one_ability(object_type *o_ptr);
enum {
    CREATE_ART_NORMAL = 0x00,
    CREATE_ART_SCROLL = 0x01,
    CREATE_ART_GOOD   = 0x02,
    CREATE_ART_CURSED = 0x04,
};
extern s32b create_artifact(object_type *o_ptr, u32b mode);
extern void curse_object(object_type *o_ptr);
extern void get_bloody_moon_flags(object_type *o_ptr);
extern void random_artifact_resistance(object_type * o_ptr, artifact_type *a_ptr);
extern bool create_named_art(int a_idx, int y, int x);
extern bool create_named_art_aux(int a_idx, object_type *o_ptr);
extern bool create_named_art_aux_aux(int a_idx, object_type *o_ptr);
extern bool create_replacement_art(int a_idx, object_type *o_ptr);
extern bool reforge_artifact(object_type *src, object_type *dest, int fame);
extern void get_random_name(char *return_name, object_type *o_ptr, int power);

/* scores.c */
extern void display_scores_aux(int from, int to, int note, high_score *score);
extern void display_scores(int from, int to);
extern void kingly(void);
extern bool send_world_score(bool do_send);
extern errr top_twenty(void);
extern errr predict_score(void);
extern void race_legends(void);
extern void race_score(int race_num);
extern void show_highclass(void);

/* mspells3.c */
extern bool do_cmd_cast_learned(void);
extern void learn_spell(int monspell);
extern void set_rf_masks(s32b *f4, s32b *f5, s32b *f6, int mode);

/* hissatsu.c */
extern void hissatsu_info(char *p, int power);
extern void do_cmd_hissatsu(void);
extern void do_cmd_hissatsu_browse(void);
extern void do_cmd_gain_hissatsu(void);

/*
 * Hack -- conditional (or "bizarre") externs
 */

#ifdef SET_UID
/* util.c */
extern void user_name(char *buf, int id);
#endif

#ifndef HAVE_USLEEP
/* util.c */
extern int usleep(huge usecs);
#endif

#ifdef MACINTOSH
/* main-mac.c */
/* extern void main(void); */
#endif

#if defined(MAC_MPW) || defined(MACH_O_CARBON)
/* Globals needed */
extern  u32b _ftype;
extern  u32b _fcreator;
#endif

#if defined(MAC_MPW) && defined(CARBON)
extern void convert_pathname(char *path);
#endif

#if defined(MACH_O_CARBON)
extern void fsetfileinfo(cptr path, u32b fcreator, u32b ftype);
#endif

/* util.c */
#ifdef ALLOW_REPEAT /* TNB ... 'n' repeats the last command */
#define REPEAT_PULL(pn) repeat_pull(pn)
#define REPEAT_PUSH(pn) repeat_push(pn)

extern int count_bits(u32b x);
extern void repeat_push(int what);
extern bool repeat_pull(int *what);
extern void repeat_check(void);

#else
#define REPEAT_PULL(pn) FALSE
#define REPEAT_PUSH(pn) ((void)0)
#endif /* ALLOW_REPEAT -- TNB */

#ifdef ALLOW_EASY_OPEN /* TNB */

/* variable.c */
extern bool easy_open;

/* cmd2.c */
extern bool easy_open_door(int y, int x);

#endif /* ALLOW_EASY_OPEN -- TNB */

#ifdef ALLOW_EASY_DISARM /* TNB */

/* variable.c */
extern bool easy_disarm;

/* cmd2.c */
extern bool do_cmd_disarm_aux(int y, int x, int dir);

#endif /* ALLOW_EASY_DISARM -- TNB */


#ifdef ALLOW_EASY_FLOOR /* TNB */

/* object1.c */
extern int scan_floor(int *items, int y, int x, int mode);
extern int show_floor(int target_item, int y, int x, int *min_width);
extern bool get_item_floor(int *cp, cptr pmt, cptr str, int mode);
extern void py_pickup_floor(bool pickup);

/* variable.c */
extern bool easy_floor;

#endif /* ALLOW_EASY_FLOOR -- TNB */

/* obj_kind.c */
extern const int pval_flags[];
extern bool have_pval_flag(u32b flgs[OF_ARRAY_SIZE]);
extern bool is_pval_flag(int which);
extern bool object_is_cloak(object_type *o_ptr);
extern bool object_is_gloves(object_type *o_ptr);
extern bool object_is_helmet(object_type *o_ptr);
extern bool object_is_cursed(object_type *o_ptr);
extern bool object_is_mushroom(object_type *o_ptr);
extern bool object_is_flavor(object_type *o_ptr);
extern bool object_is_potion(object_type *o_ptr);
extern bool mon_is_wanted(int r_idx);
extern bool object_is_shoukinkubi(object_type *o_ptr);
extern bool object_is_favorite(object_type *o_ptr);
extern bool object_is_rare(object_type *o_ptr);
extern bool object_is_weapon(object_type *o_ptr);
extern bool object_is_device(object_type *o_ptr);
extern bool object_is_bow(object_type *o_ptr);
extern bool object_is_weapon_ammo(object_type *o_ptr);
extern bool object_is_ammo(object_type *o_ptr);
extern bool object_is_armour(object_type *o_ptr);
extern bool object_is_shield(object_type *o_ptr);
extern bool object_is_body_armour(object_type *o_ptr);
extern bool object_is_ring(object_type *o_ptr);
extern bool object_is_amulet(object_type *o_ptr);
extern bool object_is_lite(object_type *o_ptr);
extern bool object_is_boots(object_type *o_ptr);
extern bool enchantment_hack;
extern bool object_is_weapon_armour_ammo(object_type *o_ptr);
extern bool object_is_melee_weapon(object_type *o_ptr);
extern bool object_is_jewelry(object_type *o_ptr);
extern bool object_is_wearable(object_type *o_ptr);
extern bool object_is_equipment(object_type *o_ptr);
extern bool object_refuse_enchant_weapon(object_type *o_ptr);
extern bool object_allow_enchant_weapon(object_type *o_ptr);
extern bool object_allow_enchant_melee_weapon(object_type *o_ptr);
extern bool object_is_smith(object_type *o_ptr);
extern bool object_is_artifact(object_type *o_ptr);
extern bool object_is_dragon_armor(object_type *o_ptr);
extern bool object_is_nameless(object_type *o_ptr);
extern bool object_is_scroll(object_type *o_ptr);
extern bool object_allow_two_hands_wielding(object_type *o_ptr);

/* wild.c */
extern void set_floor_and_wall(byte type);
extern void wilderness_gen(void);
extern monster_hook_type wilderness_mon_hook;
extern void wilderness_gen_small(void);
extern void wilderness_move_player(int old_x, int old_y);
extern bool wilderness_scroll_lock;
extern int  wilderness_level(int x, int y);
extern errr init_wilderness(void);
extern void init_wilderness_terrains(void);
extern void seed_wilderness(void);
extern errr parse_line_wilderness(char *buf, int ymin, int xmin, int ymax, int xmax, int *y, int *x);
extern bool change_wild_mode(void);

/* wizard2.c */
extern bool spoiler_hack;
extern bool statistics_hack;
extern bool character_dump_hack;
extern void strip_name(char *buf, int k_idx);
extern void strip_name_aux(char *dest, const char *src);
extern cptr race_spoiler_page(int i);
extern vec_ptr stats_rand_arts(void);
extern void stats_add_rand_art(object_type *o_ptr);
extern vec_ptr stats_egos(void);
extern void stats_add_ego(object_type *o_ptr);

/* avatar.c */
extern cptr virtue_name(int which);
extern int virtue_find(int which);
extern bool virtue_present(int which);
extern int virtue_current(int which);
extern void virtue_add(int which, int amount);
extern void virtue_init(void);
extern void virtue_display(doc_ptr doc);

extern travel_type travel;

/* variable.c (for snipers) */
extern int snipe_type;
extern bool reset_concent;   /* Concentration reset flag */
extern bool is_fired;

/* snipe.c */
extern void reset_concentration(bool msg);
extern void display_snipe_list(void);
extern int tot_dam_aux_snipe (int mult, monster_type *m_ptr);
extern void do_cmd_snipe(void);
extern void do_cmd_snipe_browse(void);
extern int boost_concentration_damage(int tdam);

/* hex.c */
extern bool stop_hex_spell_all(void);
extern bool stop_hex_spell(void);
extern void check_hex(void);
extern bool hex_spell_fully(void);
extern void hex_stop_spelling_spell(int cmd, variant *res);
extern void revenge_spell(void);
extern void revenge_store(int dam);
extern bool teleport_barrier(int m_idx, char *effectname);
extern bool magic_barrier(int m_idx);
extern bool multiply_barrier(int m_idx);

/* personalities.c */
extern personality_ptr get_personality_aux(int index);
extern personality_ptr get_personality(void);

/* races.c */
extern int get_race_idx(cptr name);
extern bool prace_is_(int which);
extern race_t *get_race(void);      /* Actual Race (cf Mimics) */
extern race_t *get_true_race(void); /* True Race */
extern race_t *get_race_aux(int prace, int psubrace);

/* Player Races */
extern void mimic_race(int new_race, const char *msg);
extern void mimic_upkeep(void);
extern bool mimic_no_regen(void);

extern cptr gf_name(int which);

extern race_t *amberite_get_race(void);

extern race_t *android_get_race(void);
extern void    android_calc_exp(void);
extern int     android_obj_exp(object_type *o_ptr);

extern race_t *archon_get_race(void);
extern race_t *balrog_get_race(void);
extern race_t *barbarian_get_race(void);
extern race_t *beastman_get_race(void);

extern race_t *centaur_get_race(void);
extern void    jump_spell(int cmd, variant *res);

extern race_t *cyclops_get_race(void);
extern race_t *dark_elf_get_race(void);
extern race_t *demigod_get_race(int psubrace);
extern void    demigod_rechoose_powers(void);
extern race_t *doppelganger_get_race(void);
extern race_t *draconian_get_race(int psubrace);
extern race_t *dunadan_get_race(void);
extern race_t *dwarf_get_race(void);
extern race_t *ent_get_race(void);
extern race_t *gnome_get_race(void);
extern race_t *golem_get_race(void);
extern race_t *half_giant_get_race(void);
extern race_t *half_ogre_get_race(void);
extern race_t *half_titan_get_race(void);
extern race_t *half_troll_get_race(void);
extern race_t *high_elf_get_race(void);
extern race_t *hobbit_get_race(void);
extern race_t *human_get_race(void);
extern race_t *imp_get_race(void);
extern race_t *klackon_get_race(void);
extern race_t *kobold_get_race(void);
extern race_t *kutar_get_race(void);
extern race_t *mindflayer_get_race(void);
extern race_t *nibelung_get_race(void);
extern race_t *shadow_fairy_get_race(void);
extern race_t *skeleton_get_race(void);
extern race_t *snotling_get_race(void);
extern race_t *spectre_get_race(void);
extern race_t *sprite_get_race(void);
extern race_t *tonberry_get_race(void);
extern race_t *vampire_get_race(void);
extern race_t *wood_elf_get_race(void);
extern race_t *yeek_get_race(void);
extern race_t *zombie_get_race(void);

extern void equip_shuffle(cptr tag); /* For shapeshifters ... */

/* Monster Races */
extern race_t *mon_angel_get_race(void);
extern race_t *mon_beholder_get_race(void);
extern race_t *mon_centipede_get_race(void);
extern race_t *mon_demon_get_race(int psubrace);
extern race_t *mon_dragon_get_race(int psubrace);
extern race_t *mon_elemental_get_race(int psubrace);
extern race_t *mon_giant_get_race(int psubrace);
extern race_t *mon_golem_get_race(int psubrace);
extern race_t *mon_hound_get_race(void);
extern race_t *mon_hydra_get_race(void);
extern race_t *mon_jelly_get_race(void);
extern race_t *mon_leprechaun_get_race(void);
extern race_t *mon_lich_get_race(void);
extern race_t *mon_mimic_get_race(void);
extern race_t *mon_possessor_get_race(void);
extern race_t *mon_quylthulg_get_race(void);
extern race_t *mon_ring_get_race(void);
extern race_t *mon_spider_get_race(void);
extern void    spider_web_spell(int cmd, variant *res);
extern race_t *mon_sword_get_race(void);
extern race_t *mon_troll_get_race(void);
extern race_t *mon_vampire_get_race(void);
extern race_t *mon_xorn_get_race(void);

extern bool dragon_vamp_hack;
extern int dragon_vamp_amt;
extern dragon_realm_ptr dragon_get_realm(int which);
extern int subjugation_power(void);
extern bool monster_toss(int m_idx);

extern void    hound_calc_innate_attacks(void);
extern void    hound_sniff_spell(int cmd, variant *res);
extern void    hound_stalk_spell(int cmd, variant *res);
extern void    hound_run_spell(int cmd, variant *res);
extern void    hound_leap_spell(int cmd, variant *res);

extern bool    possessor_can_gain_exp(void);
extern int     possessor_get_toggle(void);
extern s32b    possessor_max_exp(void);
extern void    possessor_on_take_hit(void);
extern void    possessor_on_birth(void);
extern void    possessor_calc_innate_attacks(void);
extern int     possessor_get_powers(spell_info* spells, int max);
extern int     possessor_get_spells(spell_info* spells, int max);
extern
caster_info   *possessor_caster_info(void);
extern void    possessor_calc_bonuses(void);
extern int     possessor_r_speed(int r_idx);
extern int     possessor_r_ac(int r_idx);
extern void    possessor_get_flags(u32b flgs[OF_ARRAY_SIZE]);
extern void    possessor_get_immunities(u32b flgs[OF_ARRAY_SIZE]);
extern void    possessor_get_vulnerabilities(u32b flgs[OF_ARRAY_SIZE]);
extern void    possessor_character_dump(doc_ptr doc);
extern void    possessor_on_load(savefile_ptr file);
extern void    possessor_on_save(savefile_ptr file);
extern void    possessor_set_current_r_idx(int r_idx);
extern void    possessor_explode(int dam);
extern void    possessor_init_race_t(race_t *race_ptr, int default_r_idx);
extern void    mimic_dispel_player(void);
extern void    mimic_on_kill_monster(int r_idx);

extern bool    giant_is_favorite(object_type *o_ptr);
extern void    monster_toss_spell(int cmd, variant *res);
extern bool    jelly_eat_object(object_type *o_ptr);

extern void    blink_toggle_spell(int cmd, variant *res);
extern bool    leprechaun_steal(int m_idx);
extern int     leprechaun_get_toggle(void);

extern int     sword_calc_torch(void);
extern bool    sword_disenchant(void);

extern int     ring_calc_torch(void);
extern bool    ring_disenchant(void);
extern void    ring_cast(void);
extern void    ring_browse(void);
extern bool    ring_dominate_m(int m_idx);
extern void    ring_process_m(int m_idx);
extern void    ring_summon_ring_bearer(void);

extern bool    vampiric_drain_hack;
extern void    vampire_feed(int amt);
extern void    vampire_check_light_status(void);
extern void    vampire_take_light_damage(int amt);
extern void    vampire_take_dark_damage(int amt);

/* Mimic Forms */
extern race_t *bat_get_race(void);
extern race_t *clay_golem_get_race(void);
extern race_t *colossus_get_race(void);
extern race_t *demon_get_race(void);
extern race_t *demon_lord_get_race(void);
extern race_t *iron_golem_get_race(void);
extern race_t *mist_get_race(void);
extern race_t *mithril_golem_get_race(void);
extern race_t *vampire_lord_get_race(void);
extern race_t *small_kobold_get_race(void);
extern race_t *mangy_leper_get_race(void);
extern race_t *wolf_get_race(void);


/* classes.c */
extern class_t *get_class(void);
extern class_t *get_class_aux(int pclass, int psubclass);
extern int lookup_class_idx(cptr name);
extern int get_class_idx(void);
extern caster_info *get_caster_info(void);
extern int get_powers_aux(spell_info* spells, int max, power_info* table);
extern int get_spells_aux(spell_info* spells, int max, spell_info* table);
extern void dump_spells_aux(FILE *fff, spell_info *table, int ct);




/* duelist.c */
extern cptr duelist_current_challenge(void);
extern class_t *duelist_get_class(void);
extern bool duelist_issue_challenge(void);
extern int duelist_skill_sav(int m_idx);
extern void strafing_spell(int cmd, variant *res);
extern bool nemesis_hack;    /* Actually, its in melee1.c */


/* magic_eater.c */
extern class_t *magic_eater_get_class(void);
extern bool magic_eater_regen(int percent);
extern void magic_eater_restore(void);
extern void magic_eater_restore_all(void);
extern bool magic_eater_can_regen(void);
extern int  magic_eater_regen_amt(int tval);
extern bool magic_eater_auto_id(object_type *o_ptr);
extern bool magic_eater_auto_detect_traps(void);

extern void magic_eater_browse(void);
extern void magic_eater_cast(int tval);
extern void magic_eater_gain(void);
extern bool magic_eater_hack;

/* mauler.c */
extern class_t *mauler_get_class(void);
extern bool do_blow(int type);
extern int mauler_get_toggle(void);
extern void stunning_blow_spell(int cmd, variant *res);
extern void process_maul_of_vice(void);

/* mindcrafter.c */
extern class_t *mindcrafter_get_class(void);
extern void psycho_spear_spell(int cmd, variant *res);

/* mirror_master.c */
extern class_t *mirror_master_get_class(void);
extern bool is_mirror_grid(cave_type *c_ptr);
extern void remove_all_mirrors(bool explode);
extern void remove_mirror(int y, int x);

/* monk.c */
extern void monk_double_attack_spell(int cmd, variant *res);
extern void monk_posture_spell(int cmd, variant *res);
extern int  monk_get_attack_idx(void);
extern critical_t monk_get_critical(martial_arts *ma_ptr, int hand, int mode);
extern void monk_display_attack_info(doc_ptr, int hand);


/* mystic.c */
extern class_t *mystic_get_class(int psubclass);
extern int mystic_get_toggle(void);

/* ninja.c */
extern void quick_walk_spell(int cmd, variant *res);

/* alchemist.c */
extern class_t *alchemist_get_class(void);
extern void alchemist_cast(int tval);
extern void alchemist_super_potion_effect(int sval);

/* freelancer.c -rtl-
extern int freelancer_stab_level(void);
extern void freelancer_skill_boost(void);
extern int freelancer_hp_boost(void);
extern int freelancer_sp_boost(void);
**/
extern class_t *maledict_get_class(void);
extern bool maledict_ty_protection(void);

extern class_t *archaeologist_get_class(void);
extern bool     archaeologist_is_favored_weapon(object_type *o_ptr);
extern int      archaeologist_spell_stat_idx(void);
extern class_t *archer_get_class(void);
extern class_t *bard_get_class(void);
extern void     bard_check_music(void);
extern void     bard_start_singing(int spell, int song);
extern void     bard_stop_singing(void);
extern class_t *beastmaster_get_class(void);
extern class_t *berserker_get_class(void);
extern class_t *blood_knight_get_class(void);
extern class_t *blood_mage_get_class(void);
extern class_t *blue_mage_get_class(void);
extern class_t *cavalry_get_class(void);
extern class_t *chaos_warrior_get_class(void);
extern void     chaos_warrior_reward(void);
extern class_t *devicemaster_get_class(void);
extern bool     devicemaster_desperation;
extern cptr     devicemaster_speciality_name(int psubclass);
extern cptr     devicemaster_speciality_desc(int psubclass);
extern bool     devicemaster_is_speciality(object_type *o_ptr);

extern cptr		mystic_spec_name(int psubclass);
extern cptr     mystic_spec_desc(int psubclass);
extern bool     mystic_is_speciality(object_type *o_ptr);

extern class_t *force_trainer_get_class(void);
extern class_t *high_mage_get_class(void);
extern bool     imitator_cast(bool revenge);
extern class_t *imitator_get_class(void);
extern void     spellbook_character_dump(doc_ptr doc);
extern class_t *mage_get_class(void);
extern equip_template_ptr mon_get_equip_template(void);
extern cptr     mon_name(int r_idx);

extern class_t *monk_get_class(void);
extern void     monk_posture_calc_bonuses(void);
extern void     monk_posture_calc_stats(s16b stats[MAX_STATS]);
extern void     monk_posture_get_flags(u32b flgs[OF_ARRAY_SIZE]);

extern void     monk_ac_bonus(void);
extern class_t *monster_get_class(void);
extern class_t *ninja_get_class(void);
extern class_t *paladin_get_class(void);
extern bool     player_is_monster_king(void);
extern class_t *priest_get_class(void);
extern bool     priest_is_good(void);
extern bool     priest_is_evil(void);
extern class_t *psion_get_class(void);
extern int      psion_backlash_dam(int dam);
extern void     psion_decrement_counters(void);
extern void     psion_do_mindspring(int energy);
extern bool     psion_mon_save_p(int r_idx, int power);
extern int      psion_enchant_power(void);
extern bool     psion_process_monster(int m_idx);
extern void     psion_dispel_player(void);
extern void     psion_relearn_powers(void);
extern bool     psion_archery(void);
extern bool     psion_backlash(void);
extern bool     psion_blending(void);
extern bool     psion_clarity(void);
extern bool     psion_combat(void);
extern bool     psion_mental_fortress(void);
extern bool     psion_mindspring(void);
extern bool     psion_shielding(void);
extern bool     psion_speed(void);
extern bool     psion_weapon_graft(void);
extern bool     psion_can_wield(object_type *o_ptr);
extern bool     psion_check_dispel(void);

extern bool     psion_foresight(void);
extern bool     psion_check_foresight(void);

extern bool     psion_disruption(void);
extern bool     psion_check_disruption(int m_idx);

extern bool     psion_drain(void);
extern int      psion_do_drain(int spell_idx, int dam);


extern void     rage_mage_browse_spell(void);
extern void     rage_mage_gain_spell(void);
extern class_t *rage_mage_get_class(void);
extern void     rage_mage_blood_lust(int dam);
extern void     rage_mage_rage_fueled(int dam);
extern class_t *ranger_get_class(void);
extern class_t *red_mage_get_class(void);

extern class_t *rogue_get_class(void);
extern cptr     do_burglary_spell(int spell, int mode);

extern bool     rune_add(object_type *o_ptr, int which, bool prompt);
extern cptr     rune_desc(int which);
extern void     rune_calc_bonuses(object_type *o_ptr);
extern void     rune_calc_stats(object_type *o_ptr, s16b stats[MAX_STATS]);
extern class_t *rune_knight_get_class(void);

extern void     samurai_concentration_spell(int cmd, variant *res);
extern class_t *samurai_get_class(void);
extern void     samurai_posture_spell(int cmd, variant *res);
extern void     samurai_posture_get_flags(u32b flgs[OF_ARRAY_SIZE]);
extern void     samurai_posture_calc_stats(s16b stats[MAX_STATS]);
extern void     samurai_posture_calc_bonuses(void);

extern class_t *tourist_get_class(void);
extern class_t *scout_get_class(void);
extern class_t *sniper_get_class(void);
extern class_t *sorcerer_get_class(void);

extern class_t *warlock_get_class(int psubclass);
extern bool     warlock_is_pact_monster(monster_race *r_ptr);
extern int      warlock_get_toggle(void);
extern void     warlock_stop_singing(void);

extern class_t *warrior_get_class(void);
extern class_t *warrior_mage_get_class(void);

extern void     weaponsmith_object_flags(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE]);
extern class_t *weaponsmith_get_class(void);

extern cptr do_hissatsu_spell(int spell, int mode);

/* necromancer.c */
extern bool     repose_of_the_dead;
extern cptr     do_necromancy_spell(int spell, int mode);
extern class_t *necromancer_get_class(void);
extern void     on_p_hit_m(int m_idx);

/* skills.c */
extern skill_table *s_info; /* deprecated ... in process of removing naked table reads*/
extern void skills_add(skills_t *dest, skills_t *src);
extern void skills_scale(skills_t *dest, int num, int denom);
extern void skills_init(skills_t *dest);
typedef struct { cptr desc; byte color; } skill_desc_t;
extern skill_desc_t skills_describe(int amt, int div);

extern int skills_bow_current(int sval);
extern int skills_bow_max(int sval);
extern void skills_bow_gain(int sval);

extern int skills_weapon_current(int tval, int sval);
extern int skills_weapon_max(int tval, int sval);
extern void skills_weapon_gain(int tval, int sval);
extern bool skills_weapon_is_icky(int tval, int sval);
extern int skills_weapon_calc_bonus(int tval, int sval);
extern cptr skills_weapon_describe_current(int tval, int sval);

extern int skills_shield_current(int sval);
extern int skills_shield_max(int sval);
extern void skills_shield_gain(int sval);
extern int skills_shield_calc_bonus(int sval);
extern cptr skills_shield_describe_current(int sval);

extern void skills_dual_wielding_gain(monster_race *r_ptr);
extern int skills_dual_wielding_current(void);
extern int skills_dual_wielding_max(void);

extern void skills_martial_arts_gain(void);
extern int skills_martial_arts_current(void);
extern int skills_martial_arts_max(void);

extern void skills_riding_gain_melee(monster_race *r_ptr);
extern void skills_riding_gain_archery(monster_race *r_ptr);
extern int skills_riding_current(void);
extern int skills_riding_max(void);

extern int skills_innate_current(cptr name);
extern int skills_innate_max(cptr name);
extern void skills_innate_gain(cptr name);
extern void skills_innate_init(cptr name, int current, int max);
extern int skills_innate_calc_bonus(cptr name);
extern cptr skills_innate_calc_name(innate_attack_ptr attack); /* Note: Uses a shared buffer so result valid only until the next call */
extern cptr skills_innate_describe_current(cptr name);

extern void skills_on_birth(void);
extern void skills_on_load(savefile_ptr file);
extern void skills_on_save(savefile_ptr file);


/* time_lord.c */
extern class_t *time_lord_get_class(void);
extern bool check_foresight(void);
extern bool devolve_monster(int m_idx, bool msg);
extern bool evolve_monster(int m_idx, bool msg);
extern bool mon_amnesia(int m_idx);

/* weaponmaster.c */
extern class_t *weaponmaster_get_class(int subclass);
extern int weaponmaster_get_toggle(void);
extern void weaponmaster_set_toggle(int toggle);
extern void weaponmaster_adjust_skills(void);
extern bool weaponmaster_is_favorite(object_type *o_ptr);
extern int weaponmaster_get_max_blows(object_type *o_ptr, int hand);
extern int weaponmaster_wield_hack(object_type *o_ptr);
extern void weaponmaster_do_wild_blade(void);
extern void weaponmaster_do_readied_shot(monster_type *m_ptr);
extern int shoot_hack;
extern int shoot_count;
extern int shoot_item;
extern cptr weaponmaster_speciality_name(int psubclass);

extern void circle_kick(void);

/* spoilers.c */
extern void generate_spoilers(void);


/* wild_talent.c */
extern class_t *wild_talent_get_class(void);
extern void wild_talent_scramble(void);
extern void wild_talent_new_life(void);
extern void wild_talent_fix_up(void);

/* wild_realm.c */
extern void wild_weapon_strike(void);
extern void wild_decrement_counters(void);
extern void wild_reset_counters(void);
extern void wild_dispel_player(void);
extern bool wild_has_power(int power);
extern void wild_reset_counter(int power);