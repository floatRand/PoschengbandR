/* File: bldg.c */

/*
 * Purpose: Building commands
 * Created by Ken Wigle for Kangband - a variant of Angband 2.8.3
 * -KMW-
 *
 * Rewritten for Kangband 2.8.3i using Kamband's version of
 * bldg.c as written by Ivan Tkatchev
 *
 * Changed for ZAngband by Robert Ruehlmann
 */

#include "angband.h"
#include "equip.h"

/* hack as in leave_store in store.c */
static bool leave_bldg = FALSE;

int get_bldg_member_code(cptr name)
{
    if (strcmp(name, "None") == 0)
        return BUILDING_NON_MEMBER;

    if (strcmp(name, "Owner") == 0)
        return BUILDING_OWNER;

    if (strcmp(name, "Member") == 0)
        return BUILDING_MEMBER;

    return -1;
}

static bool is_owner(building_type *bldg)
{
    int race_idx = (p_ptr->mimic_form != MIMIC_NONE) ? p_ptr->mimic_form : p_ptr->prace;

    if (bldg->member_class[p_ptr->pclass] == BUILDING_OWNER)
    {
        return (TRUE);
    }

    if (bldg->member_race[race_idx] == BUILDING_OWNER)
    {
        return (TRUE);
    }

    if ((is_magic(p_ptr->realm1) && (bldg->member_realm[p_ptr->realm1] == BUILDING_OWNER)) ||
        (is_magic(p_ptr->realm2) && (bldg->member_realm[p_ptr->realm2] == BUILDING_OWNER)))
    {
        return (TRUE);
    }

    return (FALSE);
}


static bool is_member(building_type *bldg)
{
    int race_idx = (p_ptr->mimic_form != MIMIC_NONE) ? p_ptr->mimic_form : p_ptr->prace;

    if (bldg->member_class[p_ptr->pclass])
    {
        return (TRUE);
    }

    if (bldg->member_race[race_idx])
    {
        return (TRUE);
    }

    if ((is_magic(p_ptr->realm1) && bldg->member_realm[p_ptr->realm1]) ||
        (is_magic(p_ptr->realm2) && bldg->member_realm[p_ptr->realm2]))
    {
        return (TRUE);
    }


    if (p_ptr->pclass == CLASS_SORCERER)
    {
        int i;
        bool OK = FALSE;
        for (i = 0; i < MAX_MAGIC; i++)
        {
            if (bldg->member_realm[i+1]) OK = TRUE;
        }
        return OK;
    }
    return (FALSE);
}


/*
 * Clear the building information
 */
static void clear_bldg(int min_row, int max_row)
{
    int   i;

    for (i = min_row; i <= max_row; i++)
        prt("", i, 0);
}

static void building_prt_gold(void)
{
    char tmp[10];
    char tmp_str[80];

    prt("Gold Remaining: ", 23, 53);

    big_num_display(p_ptr->au, tmp);
    sprintf(tmp_str, "%6.6s", tmp);
    prt(tmp_str, 23, 68);
}


/*
 * Display a building.
 */
static void show_building(building_type* bldg)
{
    char buff[20];
    int i;
    byte action_color;
    char tmp_str[80];

    Term_clear();
    sprintf(tmp_str, "%s (%s) %35s", bldg->owner_name, bldg->owner_race, bldg->name);
    prt(tmp_str, 3, 1);


    for (i = 0; i < 8; i++)
    {
        if (bldg->letters[i])
        {
            int factor = store_calc_price_factor(100);
            int member_cost = store_calc_sell_price(bldg->member_costs[i], factor);
            int other_cost = store_calc_sell_price(bldg->other_costs[i], factor);
            bool owner = is_owner(bldg);
            bool member = is_member(bldg);
            int cost = owner ? member_cost : other_cost;

            if (bldg->action_restr[i] == 0)
            {
                if (cost == 0)
                {
                    action_color = TERM_WHITE;
                    buff[0] = '\0';
                }
                else
                {
                    action_color = TERM_YELLOW;
                    sprintf(buff, "(%dgp)", cost);
                }
            }
            else if (bldg->action_restr[i] == 1)
            {
                if (!member)
                {
                    action_color = TERM_L_DARK;
                    strcpy(buff, "(closed)");

                }
                else if (cost == 0)
                {
                    action_color = TERM_WHITE;
                    buff[0] = '\0';
                }
                else
                {
                    action_color = TERM_YELLOW;
                    sprintf(buff, "(%dgp)", cost);
                }
            }
            else
            {
                if (!owner)
                {
                    action_color = TERM_L_DARK;
                    strcpy(buff, "(closed)");

                }
                else if (cost == 0)
                {
                    action_color = TERM_WHITE;
                    buff[0] = '\0';
                }
                else
                {
                    action_color = TERM_YELLOW;
                    sprintf(buff, "(%dgp)", cost);
                }
            }

            if (cost > p_ptr->au)
                action_color = TERM_L_DARK;

            sprintf(tmp_str," %c) %s %s", bldg->letters[i], bldg->act_names[i], buff);
            c_put_str(action_color, tmp_str, 19+(i/2), 35*(i%2));
        }
    }

    prt(" ESC) Exit building", 23, 0);

}

/*
 * arena commands
 */
static void arena_comm(int cmd)
{
    monster_race    *r_ptr;
    cptr            name;


    switch (cmd)
    {
        case BACT_ARENA:
            if (p_ptr->arena_number == MAX_ARENA_MONS)
            {
                clear_bldg(5, 19);
                prt("               Arena Victor!", 5, 0);
                prt("Congratulations!  You have defeated all before you.", 7, 0);
                prt("For that, receive the prize: 1,000,000 gold pieces", 8, 0);

                prt("", 10, 0);
                prt("", 11, 0);
                p_ptr->au += 1000000;
                stats_on_gold_winnings(1000000);
                msg_prompt("Press the space bar to continue", " ", PROMPT_NEW_LINE | PROMPT_FORCE_CHOICE);
                p_ptr->arena_number++;
            }
            else if (p_ptr->arena_number > MAX_ARENA_MONS)
            {
                if (p_ptr->arena_number < MAX_ARENA_MONS+2)
                {
                    msg_print("The strongest challenger is waiting for you.");
                    if (get_check("Do you fight? "))
                    {
                        p_ptr->exit_bldg = FALSE;
                        reset_tim_flags();

                        /* Save the surface floor as saved floor */
                        prepare_change_floor_mode(CFM_SAVE_FLOORS);

                        p_ptr->inside_arena = TRUE;
                        p_ptr->leaving = TRUE;
                        leave_bldg = TRUE;
                    }
                    else
                    {
                        msg_print("We are disappointed.");
                    }
                }
                else
                {
                    msg_print("You enter the arena briefly and bask in your glory.");
                }
            }
            else if (p_ptr->riding && p_ptr->pclass != CLASS_BEASTMASTER && p_ptr->pclass != CLASS_CAVALRY && p_ptr->prace != RACE_MON_RING)
            {
                msg_print("You don't have permission to enter with pet.");
            }
            else
            {
                p_ptr->exit_bldg = FALSE;
                reset_tim_flags();

                /* Save the surface floor as saved floor */
                prepare_change_floor_mode(CFM_SAVE_FLOORS);

                p_ptr->inside_arena = TRUE;
                p_ptr->leaving = TRUE;
                leave_bldg = TRUE;
            }
            break;
        case BACT_POSTER:
            if (p_ptr->arena_number == MAX_ARENA_MONS)
                msg_print("You are victorious. Enter the arena for the ceremony.");

            else if (p_ptr->arena_number > MAX_ARENA_MONS)
            {
                msg_print("You have won against all foes.");
            }
            else
            {
                r_ptr = &r_info[arena_info[p_ptr->arena_number].r_idx];
                name = (r_name + r_ptr->name);
                msg_format("Do I hear any challenges against: %s", name);
            }
            break;
        case BACT_ARENA_RULES:

            /* Save screen */
            screen_save();

            /* Peruse the arena help file */
            (void)show_file(TRUE, "arena.txt", NULL, 0, 0);


            /* Load screen */
            screen_load();

            break;
    }
}


/*
 * display fruit for dice slots
 */
static void display_fruit(int row, int col, int fruit)
{
    switch (fruit)
    {
        case 0: /* lemon */
            c_put_str(TERM_YELLOW, "   ####.", row, col);
            c_put_str(TERM_YELLOW, "  #    #", row + 1, col);
            c_put_str(TERM_YELLOW, " #     #", row + 2, col);
            c_put_str(TERM_YELLOW, "#      #", row + 3, col);
            c_put_str(TERM_YELLOW, "#      #", row + 4, col);
            c_put_str(TERM_YELLOW, "#     # ", row + 5, col);
            c_put_str(TERM_YELLOW, "#    #  ", row + 6, col);
            c_put_str(TERM_YELLOW, ".####   ", row + 7, col);
            prt(                   " Lemon  ", row + 8, col);

            break;
        case 1: /* orange */
            c_put_str(TERM_ORANGE, "   ##   ", row, col);
            c_put_str(TERM_ORANGE, "  #..#  ", row + 1, col);
            c_put_str(TERM_ORANGE, " #....# ", row + 2, col);
            c_put_str(TERM_ORANGE, "#......#", row + 3, col);
            c_put_str(TERM_ORANGE, "#......#", row + 4, col);
            c_put_str(TERM_ORANGE, " #....# ", row + 5, col);
            c_put_str(TERM_ORANGE, "  #..#  ", row + 6, col);
            c_put_str(TERM_ORANGE, "   ##   ", row + 7, col);
            prt(                   " Orange ", row + 8, col);

            break;
        case 2: /* sword */
            c_put_str(TERM_SLATE, "   /\\   " , row, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 1, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 2, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 3, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 4, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 5, col);
            c_put_str(TERM_UMBER, " ###### " , row + 6, col);
            c_put_str(TERM_UMBER, "   ##   " , row + 7, col);
            prt(                  " Sword  " , row + 8, col);

            break;
        case 3: /* shield */
            c_put_str(TERM_SLATE, " ###### ", row, col);
            c_put_str(TERM_SLATE, "#      #", row + 1, col);
            c_put_str(TERM_SLATE, "# ++++ #", row + 2, col);
            c_put_str(TERM_SLATE, "# +==+ #", row + 3, col);
            c_put_str(TERM_SLATE, "#  ++  #", row + 4, col);
            c_put_str(TERM_SLATE, " #    # ", row + 5, col);
            c_put_str(TERM_SLATE, "  #  #  ", row + 6, col);
            c_put_str(TERM_SLATE, "   ##   ", row + 7, col);
            prt(                  " Shield ", row + 8, col);

            break;
        case 4: /* plum */
            c_put_str(TERM_VIOLET, "   ##   ", row, col);
            c_put_str(TERM_VIOLET, " ###### ", row + 1, col);
            c_put_str(TERM_VIOLET, "########", row + 2, col);
            c_put_str(TERM_VIOLET, "########", row + 3, col);
            c_put_str(TERM_VIOLET, "########", row + 4, col);
            c_put_str(TERM_VIOLET, " ###### ", row + 5, col);
            c_put_str(TERM_VIOLET, "  ####  ", row + 6, col);
            c_put_str(TERM_VIOLET, "   ##   ", row + 7, col);
            prt(                   "  Plum  ", row + 8, col);

            break;
        case 5: /* cherry */
            c_put_str(TERM_RED, "      ##", row, col);
            c_put_str(TERM_RED, "   ###  ", row + 1, col);
            c_put_str(TERM_RED, "  #..#  ", row + 2, col);
            c_put_str(TERM_RED, "  #..#  ", row + 3, col);
            c_put_str(TERM_RED, " ###### ", row + 4, col);
            c_put_str(TERM_RED, "#..##..#", row + 5, col);
            c_put_str(TERM_RED, "#..##..#", row + 6, col);
            c_put_str(TERM_RED, " ##  ## ", row + 7, col);
            prt(                " Cherry ", row + 8, col);

            break;
    }
}

/*
 * kpoker no (tyuto-hannpa na)pakuri desu...
 * joker ha shineru node haitte masen.
 *
 * TODO: donataka! tsukutte!
 *  - agatta yaku no kiroku (like DQ).
 *  - kakkoii card no e.
 *  - sousa-sei no koujyo.
 *  - code wo wakariyasuku.
 *  - double up.
 *  - Joker... -- done.
 *
 * 9/13/2000 --Koka
 * 9/15/2000 joker wo jissou. soreto, code wo sukosi kakikae. --Habu
 */
#define SUIT_OF(card)  ((card) / 13)
#define NUM_OF(card)   ((card) % 13)
#define IS_JOKER(card) ((card) == 52)

static int cards[5]; /* tefuda no card */

static void reset_deck(int deck[])
{
    int i;
    for (i = 0; i < 53; i++) deck[i] = i;

    /* shuffle cards */
    for (i = 0; i < 53; i++){
        int tmp1 = randint0(53 - i) + i;
        int tmp2 = deck[i];
        deck[i] = deck[tmp1];
        deck[tmp1] = tmp2;
    }
}

static bool have_joker(void)
{
    int i;

    for (i = 0; i < 5; i++){
      if(IS_JOKER(cards[i])) return TRUE;
    }
    return FALSE;
}

static bool find_card_num(int num)
{
    int i;
    for (i = 0; i < 5; i++)
        if (NUM_OF(cards[i]) == num && !IS_JOKER(cards[i])) return TRUE;
    return FALSE;
}

static bool yaku_check_flush(void)
{
    int i, suit;
    bool joker_is_used = FALSE;

    suit = IS_JOKER(cards[0]) ? SUIT_OF(cards[1]) : SUIT_OF(cards[0]);
    for (i = 0; i < 5; i++){
        if (SUIT_OF(cards[i]) != suit){
          if(have_joker() && !joker_is_used)
            joker_is_used = TRUE;
          else
            return FALSE;
        }
    }

    return TRUE;
}

static int yaku_check_straight(void)
{
    int i, lowest = 99;
    bool joker_is_used = FALSE;

    /* get lowest */
    for (i = 0; i < 5; i++)
    {
        if (NUM_OF(cards[i]) < lowest && !IS_JOKER(cards[i]))
            lowest = NUM_OF(cards[i]);
    }
    
    if (yaku_check_flush())
    {
      if( lowest == 0 ){
        for (i = 0; i < 4; i++)
        {
            if (!find_card_num(9 + i)){
                if( have_joker() && !joker_is_used )
                  joker_is_used = TRUE;
                else
                  break;
            }
        }
        if (i == 4) return 3; /* Wow! Royal Flush!!! */
      }
      if( lowest == 9 ){
        for (i = 0; i < 3; i++)
        {
            if (!find_card_num(10 + i))
                break;
        }
        if (i == 3 && have_joker()) return 3; /* Wow! Royal Flush!!! */
      }
    }

    joker_is_used = FALSE;
    for (i = 0; i < 5; i++)
    {
        if (!find_card_num(lowest + i)){
          if( have_joker() && !joker_is_used )
            joker_is_used = TRUE;
          else
            return 0;
        }
    }
    
    if (yaku_check_flush())
        return 2; /* Straight Flush */

    return 1;
}

/*
 * 0:nopair 1:1 pair 2:2 pair 3:3 cards 4:full house 6:4cards
 */
static int yaku_check_pair(void)
{
    int i, i2, matching = 0;

    for (i = 0; i < 5; i++)
    {
        for (i2 = i+1; i2 < 5; i2++)
        {
            if (IS_JOKER(cards[i]) || IS_JOKER(cards[i2])) continue;
            if (NUM_OF(cards[i]) == NUM_OF(cards[i2]))
                matching++;
        }
    }

    if(have_joker()){
      switch(matching){
      case 0:
        matching = 1;
        break;
      case 1:
        matching = 3;
        break;
      case 2:
        matching = 4;
        break;
      case 3:
        matching = 6;
        break;
      case 6:
        matching = 7;
        break;
      default:
        /* don't reach */
        break;
      }
    }

    return matching;
}

#define ODDS_5A 3000
#define ODDS_5C 400
#define ODDS_RF 200
#define ODDS_SF 80
#define ODDS_4C 16
#define ODDS_FH 12
#define ODDS_FL 8
#define ODDS_ST 4
#define ODDS_3C 1
#define ODDS_2P 1

static int yaku_check(void)
{
    prt("                            ", 4, 3);

    switch(yaku_check_straight()){
    case 3: /* RF! */
        c_put_str(TERM_YELLOW, "Royal Flush",  4,  3);
        return ODDS_RF;
    case 2: /* SF! */
        c_put_str(TERM_YELLOW, "Straight Flush",  4,  3);
        return ODDS_SF;
    case 1:
        c_put_str(TERM_YELLOW, "Straight",  4,  3);
        return ODDS_ST;
    default:
        /* Not straight -- fall through */
        break;
    }

    if (yaku_check_flush())
    {

    c_put_str(TERM_YELLOW, "Flush",  4,  3);
        return ODDS_FL;
    }

    switch (yaku_check_pair())
    {
    case 1:
        c_put_str(TERM_YELLOW, "One pair",  4,  3);
        return 0;
    case 2:
        c_put_str(TERM_YELLOW, "Two pair",  4,  3);
        return ODDS_2P;
    case 3:
        c_put_str(TERM_YELLOW, "Three of a kind",  4,  3);
        return ODDS_3C;
    case 4:
        c_put_str(TERM_YELLOW, "Full house",  4,  3);
        return ODDS_FH;
    case 6:
        c_put_str(TERM_YELLOW, "Four of a kind",  4,  3);
        return ODDS_4C;
    case 7:
        if (!NUM_OF(cards[0]) || !NUM_OF(cards[1]))
        {
            c_put_str(TERM_YELLOW, "Five ace",  4,  3);
            return ODDS_5A;
        }
        else
        {
            c_put_str(TERM_YELLOW, "Five of a kind",  4,  3);
            return ODDS_5C;
        }
    default:
        break;
    }
    return 0;
}

static void display_kaeruka(int hoge, int kaeruka[])
{
    int i;
    char col = TERM_WHITE;
    for (i = 0; i < 5; i++)
    {
        if (i == hoge) col = TERM_YELLOW;
        else if(kaeruka[i]) col = TERM_WHITE;
        else col = TERM_L_BLUE;
        if(kaeruka[i])
            c_put_str(col, "Change", 14,  5+i*16);
        else
            c_put_str(col, " Stay ", 14,  5+i*16);
    }
    if (hoge > 4) col = TERM_YELLOW;
    else col = TERM_WHITE;
    c_put_str(col, "Sure", 16,  38);

    /* Hilite current option */
    if (hoge < 5) move_cursor(14, 5+hoge*16);
    else move_cursor(16, 38);
}


static void display_cards(void)
{
    int i, j;
    char suitcolor[4] = {TERM_YELLOW, TERM_L_RED, TERM_L_BLUE, TERM_L_GREEN};

    cptr suit[4] = {"[]", "qp", "<>", "db"};
    cptr card_grph[13][7] = {{"A    %s     ",
                  "     He     ",
                  "     ng     ",
                  "     ba     ",
                  "     nd     ",
                  "     %s     ",
                  "           A"},
                 {"2           ",
                  "     %s     ",
                  "            ",
                  "            ",
                  "            ",
                  "     %s     ",
                  "           2"},
                 {"3           ",
                  "     %s     ",
                  "            ",
                  "     %s     ",
                  "            ",
                  "     %s     ",
                  "           3"},
                 {"4           ",
                  "   %s  %s   ",
                  "            ",
                  "            ",
                  "            ",
                  "   %s  %s   ",
                  "           4"},
                 {"5           ",
                  "   %s  %s   ",
                  "            ",
                  "     %s     ",
                  "            ",
                  "   %s  %s   ",
                  "           5"},
                 {"6           ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "           6"},
                 {"7           ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "           7"},
                 {"8           ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "           8"},
                 {"9  %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s  9"},
                 {"10 %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s 10"},
                 {"J    /\\     ",
                  "%s   ||     ",
                  "     ||     ",
                  "     ||     ",
                  "     ||     ",
                  "   |=HH=| %s",
                  "     ][    J"},
                 {"Q  ######   ",
                  "%s#      #  ",
                  "  # ++++ #  ",
                  "  # +==+ #  ",
                  "   # ++ #   ",
                  "    #  #  %s",
                  "     ##    Q"},
                 {"K           ",
                  "%s _'~~`_   ",
                  "   jjjjj$&  ",
                  "   q q uu   ",
                  "   c    &   ",
                  "    v__/  %s",
                  "           K"}};
    cptr joker_grph[7] = {    "            ",
                  "     J      ",
                  "     O      ",
                  "     K      ",
                  "     E      ",
                  "     R      ",
                  "            "};

    for (i = 0; i < 5; i++)
    {
        prt(" +------------+ ",  5,  i*16);
    }

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 7; j++)
        {
            prt(" |",  j+6,  i*16);
            if(IS_JOKER(cards[i]))
                c_put_str(TERM_VIOLET, joker_grph[j],  j+6,  2+i*16);
            else
                c_put_str(suitcolor[SUIT_OF(cards[i])], format(card_grph[NUM_OF(cards[i])][j], suit[SUIT_OF(cards[i])], suit[SUIT_OF(cards[i])]),  j+6,  2+i*16);
            prt("| ",  j+6,  i*16+14);
        }
    }
    for (i = 0; i < 5; i++)
    {
        prt(" +------------+ ", 13,  i*16);
    }
}

static int do_poker(void)
{
    int i, k = 2;
    char cmd;
    int deck[53]; /* yamafuda : 0...52 */
    int deck_ptr = 0;
    int kaeruka[5]; /* 0:kaenai 1:kaeru */

    bool done = FALSE;
    bool kettei = TRUE;
    bool kakikae = TRUE;

    reset_deck(deck);

    for (i = 0; i < 5; i++)
    {
        cards[i] = deck[deck_ptr++];
        kaeruka[i] = 0; /* default:nokosu */
    }
    
#if 0
    /* debug:RF */
    cards[0] = 12;
    cards[1] = 0;
    cards[2] = 9;
    cards[3] = 11;
    cards[4] = 10;
#endif
#if 0
    /* debug:SF */
    cards[0] = 3;
    cards[1] = 2;
    cards[2] = 4;
    cards[3] = 6;
    cards[4] = 5;
#endif
#if 0
    /* debug:Four Cards */
    cards[0] = 0;
    cards[1] = 0 + 13 * 1;
    cards[2] = 0 + 13 * 2;
    cards[3] = 0 + 13 * 3;
    cards[4] = 51;
#endif
#if 0
    /* debug:Straight */
    cards[0] = 1;
    cards[1] = 0 + 13;
    cards[2] = 3;
    cards[3] = 2 + 26;
    cards[4] = 4;
#endif
#if 0
    /* debug */
    cards[0] = 52;
    cards[1] = 0;
    cards[2] = 1;
    cards[3] = 2;
    cards[4] = 3;
#endif

    /* suteruno wo kimeru */
    prt("Stay witch? ", 0, 0);

    display_cards();
    yaku_check();

    while (!done)
    {
        if (kakikae) display_kaeruka(k+kettei*5, kaeruka);
        kakikae = FALSE;
        cmd = inkey();
        switch (cmd)
        {
        case '6': case 'l': case 'L': case KTRL('F'):
            if (!kettei) k = (k+1)%5;
            else {k = 0;kettei = FALSE;}
            kakikae = TRUE;
            break;
        case '4': case 'h': case 'H': case KTRL('B'):
            if (!kettei) k = (k+4)%5;
            else {k = 4;kettei = FALSE;}
            kakikae = TRUE;
            break;
        case '2': case 'j': case 'J': case KTRL('N'):
            if (!kettei) {kettei = TRUE;kakikae = TRUE;}
            break;
        case '8': case 'k': case 'K': case KTRL('P'):
            if (kettei) {kettei = FALSE;kakikae = TRUE;}
            break;
        case ' ': case '\r':
            if (kettei) done = TRUE;
            else {kaeruka[k] = !kaeruka[k];kakikae = TRUE;}
            break;
        default:
            break;
        }
    }
    
    prt("",0,0);

    for (i = 0; i < 5; i++)
        if (kaeruka[i] == 1) cards[i] = deck[deck_ptr++]; /* soshite toru */

    display_cards();
    
    return yaku_check();
}
#undef SUIT_OF
#undef NUM_OF
#undef IS_JOKER
/* end of poker codes --Koka */

/*
 * gamble_comm
 */
static bool gamble_comm(int cmd)
{
    int i;
    int roll1, roll2, roll3, choice, odds, win;
    s32b wager;
    s32b maxbet;
    s32b oldgold;

    char out_val[160], tmp_str[80], again;
    cptr p;

    screen_save();

    if (cmd == BACT_GAMBLE_RULES)
    {
        /* Peruse the gambling help file */
        (void)show_file(TRUE, "gambling.txt", NULL, 0, 0);

    }
    else
    {
        /* No money */
        if (p_ptr->au < 1)
        {
            msg_print("Hey! You don't have gold - get out of here!");
            msg_print(NULL);
            screen_load();
            return FALSE;
        }

        clear_bldg(5, 23);

        maxbet = p_ptr->lev * 200;

        /* We can't bet more than we have */
        maxbet = MIN(maxbet, p_ptr->au);

        /* Get the wager */
        strcpy(out_val, "");
        sprintf(tmp_str,"Your wager (1-%d) ? ", maxbet);

        /*
         * Use get_string() because we may need more than
         * the s16b value returned by get_quantity().
         */
        if (get_string(tmp_str, out_val, 32))
        {
            /* Strip spaces */
            for (p = out_val; *p == ' '; p++);

            /* Get the wager */
            wager = atol(p);

            if (wager > p_ptr->au)
            {
                msg_print("Hey! You don't have the gold - get out of here!");

                msg_print(NULL);
                screen_load();
                return (FALSE);
            }
            else if (wager > maxbet)
            {
                msg_format("I'll take %d gold of that. Keep the rest.", maxbet);
                wager = maxbet;
            }
            else if (wager < 1)
            {
                msg_print("Ok, we'll start with 1 gold.");


                wager = 1;
            }
            msg_print(NULL);
            win = FALSE;
            odds = 0;
            oldgold = p_ptr->au;

            sprintf(tmp_str, "Gold before game: %9d", oldgold);
            prt(tmp_str, 20, 2);

            sprintf(tmp_str, "Current Wager:    %9d", wager);
            prt(tmp_str, 21, 2);

            /* Prevent savefile-scumming of the casino */
/*            Rand_quick = TRUE; */
            Rand_value = (u32b)time(NULL);

            do
            {
                p_ptr->au -= wager;
                stats_on_gold_services(wager);

                switch (cmd)
                {
                 case BACT_IN_BETWEEN: /* Game of In-Between */
                    c_put_str(TERM_GREEN, "In Between", 5, 2);

                    odds = 4;
                    win = FALSE;
                    roll1 = randint1(10);
                    roll2 = randint1(10);
                    choice = randint1(10);
                    sprintf(tmp_str, "Black die: %d       Black Die: %d", roll1, roll2);

                    prt(tmp_str, 8, 3);
                    sprintf(tmp_str, "Red die: %d", choice);

                    prt(tmp_str, 11, 14);
                    if (((choice > roll1) && (choice < roll2)) ||
                        ((choice < roll1) && (choice > roll2)))
                        win = TRUE;
                    break;
                case BACT_CRAPS:  /* Game of Craps */
                    c_put_str(TERM_GREEN, "Craps", 5, 2);

                    win = 3;
                    odds = 2;
                    roll1 = randint1(6);
                    roll2 = randint1(6);
                    roll3 = roll1 +  roll2;
                    choice = roll3;
                    sprintf(tmp_str, "First roll: %d %d    Total: %d", roll1,

                         roll2, roll3);
                    prt(tmp_str, 7, 5);
                    if ((roll3 == 7) || (roll3 == 11))
                        win = TRUE;
                    else if ((roll3 == 2) || (roll3 == 3) || (roll3 == 12))
                        win = FALSE;
                    else
                        do
                        {
                            msg_print("Hit any key to roll again");

                            msg_print(NULL);
                            roll1 = randint1(6);
                            roll2 = randint1(6);
                            roll3 = roll1 +  roll2;

                            sprintf(tmp_str, "Roll result: %d %d   Total:     %d",

                                 roll1, roll2, roll3);
                            prt(tmp_str, 8, 5);
                            if (roll3 == choice)
                                win = TRUE;
                            else if (roll3 == 7)
                                win = FALSE;
                        } while ((win != TRUE) && (win != FALSE));
                    break;

                case BACT_SPIN_WHEEL:  /* Spin the Wheel Game */
                    win = FALSE;
                    odds = 9;
                    c_put_str(TERM_GREEN, "Wheel", 5, 2);

                    prt("0  1  2  3  4  5  6  7  8  9", 7, 5);
                    prt("--------------------------------", 8, 3);
                    strcpy(out_val, "");
                    get_string("Pick a number (0-9): ", out_val, 32);

                    for (p = out_val; isspace(*p); p++);
                    choice = atol(p);
                    if (choice < 0)
                    {
                        msg_print("I'll put you down for 0.");

                        choice = 0;
                    }
                    else if (choice > 9)
                    {
                        msg_print("Ok, I'll put you down for 9.");

                        choice = 9;
                    }
                    msg_print(NULL);
                    roll1 = randint0(10);
                    sprintf(tmp_str, "The wheel spins to a stop and the winner is %d",

                        roll1);
                    prt(tmp_str, 13, 3);
                    prt("", 9, 0);
                    prt("*", 9, (3 * roll1 + 5));
                    if (roll1 == choice)
                        win = TRUE;
                    break;

                case BACT_DICE_SLOTS: /* The Dice Slots */
                    c_put_str(TERM_GREEN, "Dice Slots", 5, 2);

                    win = FALSE;
                    roll1 = randint1(21);
                    for (i=6;i>0;i--)
                    {
                        if ((roll1-i) < 1)
                        {
                            roll1 = 7-i;
                            break;
                        }
                        roll1 -= i;
                    }
                    roll2 = randint1(21);
                    for (i=6;i>0;i--)
                    {
                        if ((roll2-i) < 1)
                        {
                            roll2 = 7-i;
                            break;
                        }
                        roll2 -= i;
                    }
                    choice = randint1(21);
                    for (i=6;i>0;i--)
                    {
                        if ((choice-i) < 1)
                        {
                            choice = 7-i;
                            break;
                        }
                        choice -= i;
                    }
                    put_str("/--------------------------\\", 7, 2);
                    prt("\\--------------------------/", 17, 2);
                    display_fruit(8,  3, roll1 - 1);
                    display_fruit(8, 12, roll2 - 1);
                    display_fruit(8, 21, choice - 1);
                    if ((roll1 == roll2) && (roll2 == choice))
                    {
                        win = TRUE;
                        switch(roll1)
                        {
                        case 1:
                            odds = 5;break;
                        case 2:
                            odds = 10;break;
                        case 3:
                            odds = 20;break;
                        case 4:
                            odds = 50;break;
                        case 5:
                            odds = 200;break;
                        case 6:
                            odds = 1000;break;
                        }
                    }
                    else if ((roll1 == 1) && (roll2 == 1))
                    {
                        win = TRUE;
                        odds = 2;
                    }
                    break;
                case BACT_POKER:
                    win = FALSE;
                    odds = do_poker();
                    if (odds) win = TRUE;
                    break;
                }

                if (win)
                {
                    prt("YOU WON", 16, 37);

                    p_ptr->au += odds * wager;
                    stats_on_gold_winnings(odds * wager);
                    sprintf(tmp_str, "Payoff: %d", odds);

                    prt(tmp_str, 17, 37);
                }
                else
                {
                    prt("You Lost", 16, 37);
                    prt("", 17, 37);
                }
                sprintf(tmp_str, "Current Gold:     %9d", p_ptr->au);

                prt(tmp_str, 22, 2);
                prt("Again(Y/N)?", 18, 37);

                move_cursor(18, 52);
                again = inkey();
                prt("", 16, 37);
                prt("", 17, 37);
                prt("", 18, 37);
                if (wager > p_ptr->au)
                {
                    msg_print("Hey! You don't have the gold - get out of here!");
                    msg_print(NULL);

                    /* Get out here */
                    break;
                }
            } while ((again == 'y') || (again == 'Y'));

            /* Switch back to complex RNG */
            Rand_quick = FALSE;

            prt("", 18, 37);
            if (p_ptr->au >= oldgold)
            {
                msg_print("You came out a winner! We'll win next time, I'm sure.");
                virtue_add(VIRTUE_CHANCE, 3);
            }
            else
            {
                msg_print("You lost gold! Haha, better head home.");
                virtue_add(VIRTUE_CHANCE, -3);
            }
        }
        msg_print(NULL);
    }
    screen_load();
    return (TRUE);
}

static bool vault_aux_battle(int r_idx)
{
    int i;
    int dam = 0;

    monster_race *r_ptr = &r_info[r_idx];

    /* Decline town monsters */
/*    if (!mon_hook_dungeon(r_idx)) return FALSE; */

    /* Decline unique monsters */
/*    if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE); */
/*    if (r_ptr->flags7 & (RF7_NAZGUL)) return (FALSE); */

    if (r_ptr->flags1 & (RF1_NEVER_MOVE)) return (FALSE);
    if (r_ptr->flags2 & (RF2_MULTIPLY)) return (FALSE);
    if (r_ptr->flags2 & (RF2_QUANTUM)) return (FALSE);
    if (r_ptr->flags7 & (RF7_AQUATIC)) return (FALSE);
    if (r_ptr->flags7 & (RF7_CHAMELEON)) return (FALSE);

    for (i = 0; i < 4; i++)
    {
        if (r_ptr->blow[i].method == RBM_EXPLODE) return (FALSE);
        if (r_ptr->blow[i].effect != RBE_DR_MANA) dam += r_ptr->blow[i].d_dice;
    }
    if (!dam && !(r_ptr->flags4 & (RF4_BOLT_MASK | RF4_BEAM_MASK | RF4_BALL_MASK | RF4_BREATH_MASK)) && !(r_ptr->flags5 & (RF5_BOLT_MASK | RF5_BEAM_MASK | RF5_BALL_MASK | RF5_BREATH_MASK)) && !(r_ptr->flags6 & (RF6_BOLT_MASK | RF6_BEAM_MASK | RF6_BALL_MASK | RF6_BREATH_MASK))) return (FALSE);

    /* Okay */
    return (TRUE);
}

void battle_monsters(void)
{
    int total, i;
    int max_dl = 0;
    int mon_level;
    int power[4];
    bool tekitou;
    bool old_inside_battle = p_ptr->inside_battle;

    for (i = 0; i < max_d_idx; i++)
        if (max_dl < max_dlv[i]) max_dl = max_dlv[i];

    mon_level = randint1(MIN(max_dl, 122))+5;
    if (randint0(100) < 60)
    {
        i = randint1(MIN(max_dl, 122))+5;
        mon_level = MAX(i, mon_level);
    }
    if (randint0(100) < 30)
    {
        i = randint1(MIN(max_dl, 122))+5;
        mon_level = MAX(i, mon_level);
    }

    while (1)
    {
        total = 0;
        tekitou = FALSE;
        
        for(i=0;i<4;i++)
        {
            int r_idx, j;
            int attempt = 0;
            while (1)
            {
                attempt++;
                get_mon_num_prep(vault_aux_battle, NULL);
                p_ptr->inside_battle = TRUE;
                r_idx = get_mon_num(mon_level);
                p_ptr->inside_battle = old_inside_battle;
                if (!r_idx) continue;
                if (attempt > 1000) break;

                if ((r_info[r_idx].flags1 & RF1_UNIQUE) || (r_info[r_idx].flags7 & RF7_UNIQUE2))
                {
                    if ((r_info[r_idx].level + 10) > mon_level) continue;
                }

                for (j = 0; j < i; j++)
                    if(r_idx == battle_mon[j]) break;
                if (j<i) continue;

                break;
            }
            battle_mon[i] = r_idx;
            if (r_info[r_idx].level < 45) tekitou = TRUE;
        }

        /*
        battle_mon[0] = MON_SERPENT;
        battle_mon[1] = MON_TALOS;
        battle_mon[2] = MON_SOFTWARE_BUG;
        battle_mon[3] = MON_SOFTWARE_BUG;
        */

        for (i=0;i<4;i++)
        {
            monster_race *r_ptr = &r_info[battle_mon[i]];
            int num_taisei = count_bits(r_ptr->flagsr & (RFR_IM_ACID | RFR_IM_ELEC | RFR_IM_FIRE | RFR_IM_COLD | RFR_IM_POIS));

            if (r_ptr->flags1 & RF1_FORCE_MAXHP)
                power[i] = r_ptr->hdice * r_ptr->hside * 2;
            else
                power[i] = r_ptr->hdice * (r_ptr->hside + 1);
            power[i] = power[i] * (100 + r_ptr->level) / 100;
            if (r_ptr->speed > 110)
                power[i] = power[i] * (r_ptr->speed * 2 - 110) / 100;
            if (r_ptr->speed < 110)
                power[i] = power[i] * (r_ptr->speed - 20) / 100;
            if (num_taisei > 2)
                power[i] = power[i] * (num_taisei*2+5) / 10;
            else if (r_ptr->flags6 & RF6_INVULNER)
                power[i] = power[i] * 4 / 3;
            else if (r_ptr->flags6 & RF6_HEAL)
                power[i] = power[i] * 4 / 3;
            else if (r_ptr->flags5 & RF5_DRAIN_MANA)
                power[i] = power[i] * 11 / 10;
            if (r_ptr->flags1 & RF1_RAND_25)
                power[i] = power[i] * 9 / 10;
            if (r_ptr->flags1 & RF1_RAND_50)
                power[i] = power[i] * 9 / 10;

            switch (battle_mon[i])
            {
                case MON_GREEN_G:
                case MON_THAT_BAT:
                case MON_GHOST_Q:
                    power[i] /= 4;
                    break;
                case MON_LOST_SOUL:
                case MON_GHOST:
                    power[i] /= 2;
                    break;
                case MON_UND_BEHOLDER:
                case MON_SANTACLAUS:
                case MON_ULT_BEHOLDER:
                case MON_UNGOLIANT:
                case MON_ATLACH_NACHA:
                case MON_Y_GOLONAC:
                    power[i] = power[i] * 3 / 5;
                    break;
                case MON_ROBIN_HOOD:
                case MON_RICH:
                case MON_LICH:
                case MON_COLOSSUS:
                case MON_CRYPT_THING:
                case MON_MASTER_LICH:
                case MON_DREADMASTER:
                case MON_DEMILICH:
                case MON_SHADOWLORD:
                case MON_ARCHLICH:
                case MON_BLEYS:
                case MON_CAINE:
                case MON_JULIAN:
                case MON_VENOM_WYRM:
                case MON_MASTER_MYS:
                case MON_G_MASTER_MYS:
                    power[i] = power[i] * 3 / 4;
                    break;
                case MON_VORPAL_BUNNY:
                case MON_SHAGRAT:
                case MON_GORBAG:
                case MON_LOGRUS_MASTER:
                case MON_JURT:
                case MON_GRAV_HOUND:
                case MON_SHIMMERING_VORTEX:
                case MON_JUBJUB:
                case MON_CLUB_DEMON:
                case MON_LLOIGOR:
                case MON_NIGHTCRAWLER:
                case MON_NIGHTWALKER:
                case MON_RAPHAEL:
                case MON_SHAMBLER:
                case MON_SKY_DRAKE:
                case MON_GERARD:
                case MON_G_CTHULHU:
                case MON_SPECT_WYRM:
                case MON_BAZOOKER:
                case MON_GCWADL:
                case MON_KIRIN:
                case MON_FENGHUANG:
                    power[i] = power[i] * 4 / 3;
                    break;
                case MON_UMBER_HULK:
                case MON_FIRE_VORTEX:
                case MON_WATER_VORTEX:
                case MON_COLD_VORTEX:
                case MON_ENERGY_VORTEX:
                case MON_GACHAPIN:
                case MON_REVENANT:
                case MON_NEXUS_VORTEX:
                case MON_PLASMA_VORTEX:
                case MON_TIME_VORTEX:
                case MON_MANDOR:
                case MON_KAVLAX:
                case MON_RINALDO:
                case MON_STORMBRINGER:
                case MON_TIME_HOUND:
                case MON_PLASMA_HOUND:
                case MON_TINDALOS:
                case MON_CHAOS_VORTEX:
                case MON_AETHER_VORTEX:
                case MON_AETHER_HOUND:
                case MON_CANTORAS:
                case MON_GODZILLA:
                case MON_TARRASQUE:
                case MON_DESTROYER:
                case MON_MORGOTH:
                case MON_SERPENT:
                case MON_OROCHI:
                case MON_D_ELF_SHADE:
                case MON_MANA_HOUND:
                case MON_SHARD_VORTEX:
                case MON_BANORLUPART:
                case MON_BOTEI:
                case MON_JAIAN:
                case MON_BAHAMUT:
                case MON_WAHHA:
                    power[i] = power[i] * 3 / 2;
                    break;
                case MON_ROLENTO:
                case MON_CYBER:
                case MON_CYBER_KING:
                case MON_UNICORN_ORD:
                    power[i] = power[i] * 5 / 3;
                    break;
                case MON_ARCH_VILE:
                case MON_PHANTOM_B:
                case MON_WYRM_POWER:
                    power[i] *= 2;
                    break;
                case MON_NODENS:
                case MON_CULVERIN:
                    power[i] *= 3;
                    break;
                case MON_ECHIZEN:
                    power[i] *= 9;
                    break;
                case MON_HAGURE:
                    power[i] *= 100000;
                    break;
                default:
                    break;
            }
            total += power[i];
        }
        for (i=0;i<4;i++)
        {
            power[i] = total*60/power[i];
            if (tekitou && ((power[i] < 160) || power[i] > 1500)) break;
            if ((power[i] < 160) && randint0(20)) break;
            if (power[i] < 101) power[i] = 100 + randint1(5);
            mon_odds[i] = power[i];
        }
        if (i == 4) break;
    }
}

static bool kakutoujou(void)
{
    s32b maxbet;
    s32b wager;
    char out_val[160], tmp_str[80];
    cptr p;

    if ((game_turn - old_battle) > TURNS_PER_TICK*250)
    {
        battle_monsters();
        old_battle = game_turn;
    }

    screen_save();

    /* No money */
    if (p_ptr->au < 1)
    {
        msg_print("Hey! You don't have gold - get out of here!");

        msg_print(NULL);
        screen_load();
        return FALSE;
    }
    else
    {
        int i;

        clear_bldg(4, 10);

        prt("Monsters                                                       Odds", 4, 4);
        for (i=0;i<4;i++)
        {
            char buf[80];
            monster_race *r_ptr = &r_info[battle_mon[i]];

            sprintf(buf,"%d) %-58s  %4u.%02u", i+1, format("%s%s", r_name + r_ptr->name, (r_ptr->flags1 & RF1_UNIQUE) ? " (clone)" : ""), mon_odds[i]/100, mon_odds[i]%100);
            prt(buf, 5+i, 1);
        }

        prt("Which monster: ", 0, 0);
        while(1)
        {
            i = inkey();

            if (i == ESCAPE)
            {
                screen_load();
                return FALSE;
            }
            if (i >= '1' && i <= '4')
            {
                sel_monster = i-'1';
                battle_odds = mon_odds[sel_monster];
                break;
            }
            else bell();
        }

        clear_bldg(4,4);
        for (i=0;i<4;i++)
            if (i !=sel_monster) clear_bldg(i+5,i+5);

        maxbet = p_ptr->lev * 200;

        /* We can't bet more than we have */
        maxbet = MIN(maxbet, p_ptr->au);

        /* Get the wager */
        strcpy(out_val, "");
        sprintf(tmp_str,"Your wager (1-%d) ? ", maxbet);

        /*
         * Use get_string() because we may need more than
         * the s16b value returned by get_quantity().
         */
        if (get_string(tmp_str, out_val, 32))
        {
            /* Strip spaces */
            for (p = out_val; *p == ' '; p++);

            /* Get the wager */
            wager = atol(p);

            if (wager > p_ptr->au)
            {
                msg_print("Hey! You don't have the gold - get out of here!");

                msg_print(NULL);
                screen_load();
                return (FALSE);
            }
            else if (wager > maxbet)
            {
                msg_format("I'll take %d gold of that. Keep the rest.", maxbet);
                wager = maxbet;
            }
            else if (wager < 1)
            {
                msg_print("Ok, we'll start with 1 gold.");


                wager = 1;
            }
            msg_print(NULL);
            battle_odds = MAX(wager+1, wager * battle_odds / 100);
            kakekin = wager;
            p_ptr->au -= wager;
            stats_on_gold_services(wager);

            reset_tim_flags();

            /* Save the surface floor as saved floor */
            prepare_change_floor_mode(CFM_SAVE_FLOORS);

            p_ptr->inside_battle = TRUE;
            p_ptr->leaving = TRUE;

            leave_bldg = TRUE;
            screen_load();

            return (TRUE);
        }
    }
    screen_load();

    return (FALSE);
}

static void today_target(void)
{
    char buf[160];
    monster_race *r_ptr = &r_info[today_mon];

    clear_bldg(4,18);
    prt("Wanted monster that changes from day to day", 5, 10);
    sprintf(buf,"target: %s",r_name + r_ptr->name);
    c_put_str(TERM_YELLOW, buf, 6, 10);
    sprintf(buf,"corpse   ---- $%d",r_ptr->level * 50 + 100);
    prt(buf, 8, 10);
    sprintf(buf,"skeleton ---- $%d",r_ptr->level * 30 + 60);
    prt(buf, 9, 10);
    p_ptr->today_mon = today_mon;
}

static void tsuchinoko(void)
{
    clear_bldg(4,18);
c_put_str(TERM_YELLOW, "Big chance to quick money!!!", 5, 10);
c_put_str(TERM_YELLOW, "target: the rarest animal 'Tsuchinoko'", 6, 10);
c_put_str(TERM_WHITE, "catch alive ---- $1,000,000", 8, 10);
c_put_str(TERM_WHITE, "corpse      ----   $200,000", 9, 10);
c_put_str(TERM_WHITE, "bones       ----   $100,000", 10, 10);
}

static void shoukinkubi(void)
{
    int i;
    int y = 0;

    clear_bldg(4,18);

    prt("Offer a prize when you bring a wanted monster's corpse",4 ,10);
c_put_str(TERM_YELLOW, "Wanted monsters", 6, 10);

    for (i = 0; i < MAX_KUBI; i++)
    {
        byte color;
        cptr done_mark;
        monster_race *r_ptr = &r_info[(kubi_r_idx[i] > 10000 ? kubi_r_idx[i] - 10000 : kubi_r_idx[i])];

        if (kubi_r_idx[i] > 10000)
        {
            color = TERM_RED;
            done_mark = "(done)";
        }
        else
        {
            color = TERM_WHITE;
            done_mark = "";
        }

        c_prt(color, format("%s %s", r_name + r_ptr->name, done_mark), y+7, 10);

        y = (y+1) % 10;
        if (!y && (i < MAX_KUBI -1))
        {
            prt("Hit any key.", 0, 0);
            (void)inkey();
            prt("", 0, 0);
            clear_bldg(7,18);
        }
    }
}


/* List of prize object */
static struct {
    s16b tval;
    s16b sval;
} prize_list[MAX_KUBI] = 
{
    {TV_POTION, SV_POTION_CURING},
    {TV_POTION, SV_POTION_SPEED},
    {TV_POTION, SV_POTION_SPEED},
    {TV_POTION, SV_POTION_RESISTANCE},
    {TV_POTION, SV_POTION_ENLIGHTENMENT},

    {TV_POTION, SV_POTION_HEALING},
    {TV_POTION, SV_POTION_RESTORE_MANA},
    {TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION},
    {TV_POTION, SV_POTION_STAR_ENLIGHTENMENT},
    {TV_SCROLL, SV_SCROLL_CRAFTING},

    {TV_SCROLL, SV_SCROLL_GENOCIDE},
    {TV_POTION, SV_POTION_STAR_HEALING},
    {TV_POTION, SV_POTION_STAR_HEALING},
    {TV_POTION, SV_POTION_NEW_LIFE},
    {TV_SCROLL, SV_SCROLL_MASS_GENOCIDE},

    {TV_POTION, SV_POTION_LIFE},
    {TV_POTION, SV_POTION_LIFE},
    {TV_POTION, SV_POTION_AUGMENTATION},
    {TV_POTION, SV_POTION_INVULNERABILITY},
    {TV_SCROLL, SV_SCROLL_ARTIFACT},
};


/* Get prize */
static bool kankin(void)
{
    int i, j;
    bool change = FALSE;
    char o_name[MAX_NLEN];
    object_type *o_ptr;

    /* Loop for inventory and right/left arm */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        o_ptr = &inventory[i];

        /* Living Tsuchinoko worthes $1000000 */
        if ((o_ptr->tval == TV_CAPTURE) && (o_ptr->pval == MON_TSUCHINOKO))
        {
            char buf[MAX_NLEN+20];
            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            sprintf(buf, "Convert %s into money? ",o_name);
            if (get_check(buf))
            {
                int amt = 1000000 * o_ptr->number;
                msg_format("You get %dgp.", amt);
                p_ptr->au += amt;
                stats_on_gold_winnings(amt);
                p_ptr->redraw |= (PR_GOLD);
                inven_item_increase(i, -o_ptr->number);
                inven_item_describe(i);
                inven_item_optimize(i);
            }
            change = TRUE;
        }
    }

    for (i = 0; i < INVEN_PACK; i++)
    {
        o_ptr = &inventory[i];

        /* Corpse of Tsuchinoko worthes $200000 */
        if ((o_ptr->tval == TV_CORPSE) && (o_ptr->sval == SV_CORPSE) && (o_ptr->pval == MON_TSUCHINOKO))
        {
            char buf[MAX_NLEN+20];
            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            sprintf(buf, "Convert %s into money? ",o_name);
            if (get_check(buf))
            {
                int amt = 200000 * o_ptr->number;
                msg_format("You get %dgp.", amt);
                p_ptr->au += amt;
                stats_on_gold_winnings(amt);
                p_ptr->redraw |= (PR_GOLD);
                inven_item_increase(i, -o_ptr->number);
                inven_item_describe(i);
                inven_item_optimize(i);
            }
            change = TRUE;
        }
    }

    for (i = 0; i < INVEN_PACK; i++)
    {
        o_ptr = &inventory[i];

        /* Bones of Tsuchinoko worthes $100000 */
        if ((o_ptr->tval == TV_CORPSE) && (o_ptr->sval == SV_SKELETON) && (o_ptr->pval == MON_TSUCHINOKO))
        {
            char buf[MAX_NLEN+20];
            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            sprintf(buf, "Convert %s into money? ",o_name);
            if (get_check(buf))
            {
                int amt = 100000 * o_ptr->number;
                msg_format("You get %dgp.", amt);
                p_ptr->au += amt;
                stats_on_gold_winnings(amt);
                p_ptr->redraw |= (PR_GOLD);
                inven_item_increase(i, -o_ptr->number);
                inven_item_describe(i);
                inven_item_optimize(i);
            }
            change = TRUE;
        }
    }

    for (i = 0; i < INVEN_PACK; i++)
    {
        o_ptr = &inventory[i];
        if ((o_ptr->tval == TV_CORPSE) && (o_ptr->sval == SV_CORPSE) && (streq(r_name + r_info[o_ptr->pval].name, r_name + r_info[today_mon].name)))
        {
            char buf[MAX_NLEN+20];
            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            sprintf(buf, "Convert %s into money? ",o_name);
            if (get_check(buf))
            {
                int amt = (int)(r_info[today_mon].level * 50 + 100) * o_ptr->number;
                msg_format("You get %dgp.", amt);
                p_ptr->au += amt;
                stats_on_gold_winnings(amt);
                p_ptr->redraw |= (PR_GOLD);
                inven_item_increase(i, -o_ptr->number);
                inven_item_describe(i);
                inven_item_optimize(i);
            }
            change = TRUE;
        }
    }

    for (i = 0; i < INVEN_PACK; i++)
    {
        o_ptr = &inventory[i];

        if ((o_ptr->tval == TV_CORPSE) && (o_ptr->sval == SV_SKELETON) && (streq(r_name + r_info[o_ptr->pval].name, r_name + r_info[today_mon].name)))
        {
            char buf[MAX_NLEN+20];
            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            sprintf(buf, "Convert %s into money? ",o_name);
            if (get_check(buf))
            {
                int amt = (int)(r_info[today_mon].level * 30 + 60) * o_ptr->number;
                msg_format("You get %dgp.", amt);
                p_ptr->au += amt;
                stats_on_gold_winnings(amt);
                p_ptr->redraw |= (PR_GOLD);
                inven_item_increase(i, -o_ptr->number);
                inven_item_describe(i);
                inven_item_optimize(i);
            }
            change = TRUE;
        }
    }

    for (j = 0; j < MAX_KUBI; j++)
    {
        /* Need reverse order --- Positions will be changed in the loop */
        for (i = INVEN_PACK-1; i >= 0; i--)
        {
            o_ptr = &inventory[i];
            if ((o_ptr->tval == TV_CORPSE) && (o_ptr->pval == kubi_r_idx[j]))
            {
                char buf[MAX_NLEN+20];
                int num, k, item_new;
                object_type forge;

                object_desc(o_name, o_ptr, OD_COLOR_CODED);
                sprintf(buf, "Hand %s over? ",o_name);
                if (!get_check(buf)) continue;

                /* Hand it first */
                inven_item_increase(i, -o_ptr->number);
                inven_item_describe(i);
                inven_item_optimize(i);

                virtue_add(VIRTUE_JUSTICE, 5);
                p_ptr->fame++;
                kubi_r_idx[j] += 10000;

                /* Count number of unique corpses already handed */
                for (num = 0, k = 0; k < MAX_KUBI; k++)
                {
                    if (kubi_r_idx[k] >= 10000) num++;
                }

                msg_format("You earned %d point%s total.", num, (num > 1 ? "s" : ""));

                /* Prepare to make a prize */
                object_prep(&forge, lookup_kind(prize_list[num-1].tval, prize_list[num-1].sval));
                apply_magic(&forge, object_level, AM_NO_FIXED_ART);
                mass_produce(&forge);

                /* Identify it fully */
                obj_identify_fully(&forge);

                /*
                 * Hand it --- Assume there is an empty slot.
                 * Since a corpse is handed at first,
                 * there is at least one empty slot.
                 */
                item_new = inven_carry(&forge);

                /* Describe the object */
                object_desc(o_name, &forge, OD_COLOR_CODED);
                msg_format("You get %s (%c). ", o_name, index_to_label(item_new));

                /* Auto-inscription */
                autopick_alter_item(item_new, FALSE);

                /* Handle stuff */
                handle_stuff();

                change = TRUE;
            }
        }
    }

    if (!change)
    {
        msg_print("You have nothing.");
        msg_print(NULL);
        return FALSE;
    }
    return TRUE;
}

bool get_nightmare(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    /* Require eldritch horrors */
    if (!(r_ptr->flags2 & (RF2_ELDRITCH_HORROR))) return (FALSE);

    /* Require high level */
    if (r_ptr->level <= p_ptr->lev) return (FALSE);

    /* Accept this monster */
    return (TRUE);
}


void have_nightmare(int r_idx)
{
    bool happened = FALSE;
    monster_race *r_ptr = &r_info[r_idx];
    int power = r_ptr->level + 10;
    char m_name[80];
    cptr desc = r_name + r_ptr->name;

    /* Describe it */
    if (!(r_ptr->flags1 & RF1_UNIQUE))
        sprintf(m_name, "%s %s", (is_a_vowel(desc[0]) ? "an" : "a"), desc);
    else
        sprintf(m_name, "%s", desc);

    if (!(r_ptr->flags1 & RF1_UNIQUE))
    {
        if (r_ptr->flags1 & RF1_FRIENDS) power /= 2;
    }
    else power *= 2;

    if (saving_throw(p_ptr->skills.sav * 100 / power))
    {
        msg_format("%^s chases you through your dreams.", m_name);

        /* Safe */
        return;
    }

    if (p_ptr->image)
    {
        /* Something silly happens... */
        msg_format("You behold the %s visage of %s!",

                      funny_desc[randint0(MAX_SAN_FUNNY)], m_name);

        if (one_in_(3))
        {
            msg_print(funny_comments[randint0(MAX_SAN_COMMENT)]);
            p_ptr->image = p_ptr->image + randint1(r_ptr->level);
        }

        /* Never mind; we can't see it clearly enough */
        return;
    }

    /* Something frightening happens... */
    msg_format("You behold the %s visage of %s!",

                  horror_desc[randint0(MAX_SAN_HORROR)], desc);

    mon_lore_aux_2(r_ptr, RF2_ELDRITCH_HORROR);

    if (p_ptr->mimic_form != MIMIC_NONE)
    {
        switch (p_ptr->prace)
        {
        /* Demons may make a saving throw */
        case RACE_IMP:
        case RACE_BALROG:
            if (saving_throw(20 + p_ptr->lev)) return;
            break;
        /* Undead may make a saving throw */
        case RACE_SKELETON:
        case RACE_ZOMBIE:
        case RACE_SPECTRE:
        case RACE_VAMPIRE:
            if (saving_throw(10 + p_ptr->lev)) return;
            break;
        }
    }
    else
    {
        race_t *race_ptr = get_race();
        /* Demons may make a saving throw */
        if (race_ptr->flags & RACE_IS_DEMON)
        {
            if (saving_throw(20 + p_ptr->lev)) return;
        }
        /* Undead may make a saving throw */
        else if (race_ptr->flags & RACE_IS_UNDEAD)
        {
            if (saving_throw(10 + p_ptr->lev)) return;
        }
    }

    if (p_ptr->no_eldritch) return;

    /* Mind blast */
    if (!saving_throw(p_ptr->skills.sav * 100 / power))
    {
        if (!res_save_default(RES_CONF))
        {
            (void)set_confused(p_ptr->confused + randint0(4) + 4, FALSE);
        }
        if (!res_save_default(RES_CHAOS) && one_in_(3))
        {
            (void)set_image(p_ptr->image + randint0(250) + 150, FALSE);
        }
        return;
    }

    /* Lose int & wis */
    if (!saving_throw(p_ptr->skills.sav * 100 / power))
    {
        do_dec_stat(A_INT);
        do_dec_stat(A_WIS);
        return;
    }

    /* Brain smash */
    if (!saving_throw(p_ptr->skills.sav * 100 / power))
    {
        if (!res_save_default(RES_CONF))
        {
            (void)set_confused(p_ptr->confused + randint0(4) + 4, FALSE);
        }
        if (!p_ptr->free_act)
        {
            (void)set_paralyzed(randint1(4), FALSE);
        }
        while (!saving_throw(p_ptr->skills.sav))
        {
            (void)do_dec_stat(A_INT);
        }
        while (!saving_throw(p_ptr->skills.sav))
        {
            (void)do_dec_stat(A_WIS);
        }
        if (!res_save_default(RES_CHAOS))
        {
            (void)set_image(p_ptr->image + randint0(250) + 150, FALSE);
        }
        return;
    }


    /* Amnesia */
    if (!saving_throw(p_ptr->skills.sav * 100 / power))
    {
        if (lose_all_info())
        {
            msg_print("You forget everything in your utmost terror!");

        }
        return;
    }

    /* Else gain permanent insanity */
    if (mut_present(MUT_MORONIC) && mut_present(MUT_BERS_RAGE) &&
        (mut_present(MUT_COWARDICE) || res_save_default(RES_FEAR)) &&
        (mut_present(MUT_HALLUCINATION) || res_save_default(RES_CHAOS)))
    {
        /* The poor bastard already has all possible insanities! */
        return;
    }

    while (!happened)
    {
        switch (randint1(4))
        {
            case 1:
            {
                if (!mut_present(MUT_MORONIC))
                {
                    mut_gain(MUT_MORONIC);
                    happened = TRUE;
                }
                break;
            }
            case 2:
            {
                if (!mut_present(MUT_COWARDICE) && !res_save_default(RES_FEAR))
                {
                    mut_gain(MUT_COWARDICE);
                    happened = TRUE;
                }
                break;
            }
            case 3:
            {
                if (!mut_present(MUT_HALLUCINATION) && !res_save_default(RES_CHAOS))
                {
                    mut_gain(MUT_HALLUCINATION);
                    happened = TRUE;
                }
                break;
            }
            default:
            {
                if (!mut_present(MUT_BERS_RAGE))
                {
                    mut_gain(MUT_BERS_RAGE);
                    happened = TRUE;
                }
                break;
            }
        }
    }

    p_ptr->update |= PU_BONUS;
    handle_stuff();
}


/*
 * inn commands
 * Note that resting for the night was a perfect way to avoid player
 * ghosts in the town *if* you could only make it to the inn in time (-:
 * Now that the ghosts are temporarily disabled in 2.8.X, this function
 * will not be that useful. I will keep it in the hopes the player
 * ghost code does become a reality again. Does help to avoid filthy urchins.
 * Resting at night is also a quick way to restock stores -KMW-
 */
static bool inn_comm(int cmd)
{
    switch (cmd)
    {
        case BACT_FOOD: /* Buy food & drink */
            if (p_ptr->food >= PY_FOOD_FULL)
            {
                msg_print("You are full now.");
                return FALSE;
            }

            msg_print("The barkeep gives you some gruel and a beer.");

            (void)set_food(PY_FOOD_MAX - 1);
            break;

        case BACT_REST: /* Rest for the night */
            if ((p_ptr->poisoned) || (p_ptr->cut))
            {
                msg_print("You need a healer, not a room.");
                msg_print("Sorry, but I don't want anyone dying in here.");
            }
            else
            {
                s32b oldturn = game_turn;
                int prev_day, prev_hour, prev_min;

                extract_day_hour_min(&prev_day, &prev_hour, &prev_min);
                game_turn = (game_turn / (TURNS_PER_TICK*TOWN_DAWN/2) + 1) * (TURNS_PER_TICK*TOWN_DAWN/2);
                if (dungeon_turn < dungeon_turn_limit)
                {
                    dungeon_turn += MIN(game_turn - oldturn, TURNS_PER_TICK*250);
                    if (dungeon_turn > dungeon_turn_limit) dungeon_turn = dungeon_turn_limit;
                }

                prevent_turn_overflow();

                p_ptr->chp = p_ptr->mhp;

                if (p_ptr->nightmare_mode)
                {
                    msg_print("Horrible visions flit through your mind as you sleep.");
                    get_mon_num_prep(get_nightmare, NULL);
                    while(1)
                    {
                        have_nightmare(get_mon_num(MAX_DEPTH));
                        if (!one_in_(3)) break;
                    }

                    get_mon_num_prep(NULL, NULL);
                    msg_print("You awake screaming.");
                }
                else
                {
                    int i;

                    set_blind(0, TRUE);
                    set_confused(0, TRUE);
                    p_ptr->stun = 0;
                    p_ptr->chp = p_ptr->mhp;
                    if (p_ptr->pclass != CLASS_RUNE_KNIGHT)
                        p_ptr->csp = p_ptr->msp;

                    if (p_ptr->pclass == CLASS_MAGIC_EATER)
                        magic_eater_restore_all();

                    for (i = 0; i < INVEN_TOTAL; i++)
                    {
                        object_type *o_ptr = &inventory[i];

                        if (!o_ptr->k_idx) continue;
                        if (object_is_device(o_ptr))
                            device_regen_sp_aux(o_ptr, 1000);
                        else if (o_ptr->timeout > 0)
                            o_ptr->timeout = 0;
                    }

                    if (prev_hour >= 6 && prev_hour <= 17)
                        msg_print("You awake refreshed for the evening.");
                    else
                        msg_print("You awake refreshed for the new day.");
                }
            }
            break;

        case BACT_RUMORS: /* Listen for rumors */
            {
                char Rumor[1024];

                if (!get_rnd_line("rumors.txt", 0, Rumor))

                    msg_format("%s", Rumor);
                break;
            }
    }

    return (TRUE);
}


/*
 * Display quest information
 */
static void get_questinfo(int questnum)
{
    int     i;
    int     old_quest;
    char    tmp_str[80];


    /* Clear the text */
    for (i = 0; i < 10; i++)
    {
        quest_text[i][0] = '\0';
    }

    quest_text_line = 0;

    /* Set the quest number temporary */
    old_quest = p_ptr->inside_quest;
    p_ptr->inside_quest = questnum;

    /* Get the quest text */
    init_flags = INIT_SHOW_TEXT | INIT_ASSIGN;

    process_dungeon_file("q_info.txt", 0, 0, 0, 0);

    /* Reset the old quest number */
    p_ptr->inside_quest = old_quest;

    /* Print the quest info */
    sprintf(tmp_str, "Quest Information (Danger level: %d)", quest[questnum].level);

    prt(tmp_str, 5, 0);

    prt(quest[questnum].name, 7, 0);

    for (i = 0; i < 10; i++)
    {
        c_put_str(TERM_YELLOW, quest_text[i], i + 8, 0);
    }
}


/*
 * Request a quest from the Lord.
 */
static void castle_quest(void)
{
    int             q_index = 0;
    monster_race    *r_ptr;
    quest_type      *q_ptr;
    cptr            name;


    clear_bldg(4, 18);

    /* Current quest of the building */
    q_index = cave[py][px].special;

    /* Is there a quest available at the building? */
    if (!q_index)
    {
        put_str("I don't have a quest for you at the moment.", 8, 0);
        return;
    }

    q_ptr = &quest[q_index];

    /* Quest is completed */
    if (q_ptr->status == QUEST_STATUS_COMPLETED)
    {
        /* Rewarded quest */
        q_ptr->status = QUEST_STATUS_REWARDED;

        get_questinfo(q_index);

        reinit_wilderness = TRUE;
    }
    /* Failed quest */
    else if (q_ptr->status == QUEST_STATUS_FAILED)
    {
        get_questinfo(q_index);

        /* Mark quest as done (but failed) */
        q_ptr->status = QUEST_STATUS_FAILED_DONE;

        reinit_wilderness = TRUE;
    }
    /* Quest is still unfinished */
    else if (q_ptr->status == QUEST_STATUS_TAKEN)
    {
        put_str("You have not completed your current quest yet!", 8, 0);

        put_str("Use CTRL-Q to check the status of your quest.", 9, 0);

        put_str("Return when you have completed your quest.", 12, 0);
    }
    /* No quest yet */
    else if (q_ptr->status == QUEST_STATUS_UNTAKEN)
    {
        q_ptr->status = QUEST_STATUS_TAKEN;

        /* Handle $RANDOM_ for each quest independently */
        q_ptr->seed = randint0(0x10000000);

        reinit_wilderness = TRUE;

        /* Assign a new quest */
        if (q_ptr->type == QUEST_TYPE_KILL_ANY_LEVEL)
        {
            if (q_ptr->r_idx == 0)
            {
                /* Random monster at least 5 - 10 levels out of deep */
                q_ptr->r_idx = get_mon_num(q_ptr->level + 4 + randint1(6));
            }

            r_ptr = &r_info[q_ptr->r_idx];

            while ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->rarity != 1))
            {
                q_ptr->r_idx = get_mon_num(q_ptr->level) + 4 + randint1(6);
                r_ptr = &r_info[q_ptr->r_idx];
            }

            if (q_ptr->max_num == 0)
            {
                /* Random monster number */
                if (randint1(10) > 7)
                    q_ptr->max_num = 1;
                else
                    q_ptr->max_num = randint1(3) + 1;
            }

            q_ptr->cur_num = 0;
            name = (r_name + r_ptr->name);
            msg_format("Your quest: kill %d %s", q_ptr->max_num, name);

        }
        else
        {
            get_questinfo(q_index);
        }
    }
}


/*
 * Display town history
 */
static void town_history(void)
{
    /* Save screen */
    screen_save();

    /* Peruse the building help file */
    (void)show_file(TRUE, "bldg.txt", NULL, 0, 0);


    /* Load screen */
    screen_load();
}


/*
 * Hook to specify "ammo"
 */
static bool item_tester_hook_ammo(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}


/*
 * Evaluate AC
 *
 * Calculate and display the dodge-rate and the protection-rate
 * based on AC
 */
static bool eval_ac(int iAC)
{
    const char memo[] =
        "'Protection Rate' means how much damage is reduced by your armor.\n"
        "Note that the Protection rate is effective only against normal "
        "'attack' and 'shatter' type melee attacks, "
        "and has no effect against any other types such as 'poison'.\n \n"
        "'Dodge Rate' indicates the success rate on dodging the "
        "monster's melee attacks. "
        "It is depend on the level of the monster and your AC.\n \n"
        "'Average Damage' indicates the expected amount of damage "
        "when you are attacked by normal melee attacks with power=60.";

    int protection;
    int col, row = 2;
    int lvl;
    char buf[80*20], *t;

    /* AC lower than zero has no effect */
    if (iAC < 0) iAC = 0;

    protection = 100 * MIN(iAC, 150) / 250;

    screen_save();
    clear_bldg(0, 22);

    put_str(format("Your current AC : %3d", iAC), row++, 0);
    put_str(format("Protection rate : %3d%%", protection), row++, 0);
    row++;

    put_str("Level of Monster:", row + 0, 0);
    put_str("Dodge Rate      :", row + 1, 0);
    put_str("Average Damage  :", row + 2, 0);
    
    for (col = 17 + 1, lvl = 0; lvl <= 100; lvl += 10, col += 5)
    {
        int quality = 60 + lvl * 3; /* attack quality with power 60 */
        int dodge;
        int average;

        put_str(format("%3d", lvl), row + 0, col);

        dodge = 5 + (MIN(100, 100 * (iAC * 3 / 4) / quality) * 9 + 5) / 10;
        put_str(format("%3d%%", dodge), row + 1, col);

        average = (100 - dodge) * (100 - protection) / 100;
        put_str(format("%3d", average), row + 2, col);
    }

    /* Display note */
    roff_to_buf(memo, 70, buf, sizeof(buf));
    for (t = buf; t[0]; t += strlen(t) + 1)
        put_str(t, (row++) + 6, 4);

    prt("Defense abilities from your current Armor Class are evaluated below.", 0, 0);
  
    flush();
    (void)inkey();
    screen_load();

    /* Done */
    return (TRUE);
}

typedef struct _gamble_shop_s {
    int tval;
    int sval;
    int prob;
} _gamble_shop_t;

const _gamble_shop_t _gamble_shop_potions[] = {
    { TV_POTION, SV_POTION_SPEED, 50},
    { TV_POTION, SV_POTION_CURING, 50},
    { TV_POTION, SV_POTION_RESISTANCE, 30},
    { TV_POTION, SV_POTION_HEALING, 50},
    { TV_POTION, SV_POTION_STAR_HEALING, 20},
    { TV_POTION, SV_POTION_RESTORE_MANA, 20},
    { TV_POTION, SV_POTION_INC_STR, 3},
    { TV_POTION, SV_POTION_INC_INT, 3},
    { TV_POTION, SV_POTION_INC_WIS, 3},
    { TV_POTION, SV_POTION_INC_DEX, 3},
    { TV_POTION, SV_POTION_INC_CON, 3},
    { TV_POTION, SV_POTION_INC_CHR, 3},
    { TV_POTION, SV_POTION_POLYMORPH, 3},
    { TV_POTION, SV_POTION_NEW_LIFE, 3},
    { TV_POTION, SV_POTION_LIFE, 3},
    { TV_POTION, SV_POTION_STONE_SKIN, 1},
    { TV_POTION, SV_POTION_INVULNERABILITY, 1},
    { TV_POTION, SV_POTION_AUGMENTATION, 1},
    { TV_POTION, SV_POTION_HEROISM, 50},
    { TV_POTION, SV_POTION_BOLDNESS, 50},
    { TV_POTION, SV_POTION_CURE_LIGHT, 50},
    { TV_POTION, SV_POTION_CURE_SERIOUS, 50},
    { TV_POTION, SV_POTION_CURE_CRITICAL, 50},
    { TV_POTION, SV_POTION_RESTORE_EXP, 50},
    { TV_POTION, SV_POTION_DETECT_INVIS, 50},
    { TV_POTION, SV_POTION_RES_STR, 50},
    { TV_POTION, SV_POTION_RES_INT, 50},
    { TV_POTION, SV_POTION_RES_WIS, 50},
    { TV_POTION, SV_POTION_RES_DEX, 50},
    { TV_POTION, SV_POTION_RES_CON, 50},
    { TV_POTION, SV_POTION_RES_CHR, 50},
    { TV_POTION, SV_POTION_CLARITY, 50},
    { TV_POTION, SV_POTION_RESIST_HEAT, 50},
    { TV_POTION, SV_POTION_RESIST_COLD, 50},
    { 0, 0, 0}
};

const _gamble_shop_t _gamble_shop_scrolls[] = {
    { TV_SCROLL, SV_SCROLL_TELEPORT_LEVEL, 39},
    { TV_SCROLL, SV_SCROLL_RUNE_OF_PROTECTION, 30},
    { TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION, 30},
    { TV_SCROLL, SV_SCROLL_ACQUIREMENT, 15},
    { TV_SCROLL, SV_SCROLL_MASS_GENOCIDE, 20},
    { TV_SCROLL, SV_SCROLL_GENOCIDE, 20},
    { TV_SCROLL, SV_SCROLL_STAR_ACQUIREMENT, 5},
    { TV_SCROLL, SV_SCROLL_CRAFTING, 10},
    { TV_SCROLL, SV_SCROLL_MADNESS, 10},
    { TV_SCROLL, SV_SCROLL_ARTIFACT, 1},
    { TV_SCROLL, SV_SCROLL_PHASE_DOOR, 50},
    { TV_SCROLL, SV_SCROLL_TELEPORT, 50},
    { TV_SCROLL, SV_SCROLL_REMOVE_CURSE, 50},
    { TV_SCROLL, SV_SCROLL_STAR_ENCHANT_WEAPON, 50},
    { TV_SCROLL, SV_SCROLL_STAR_ENCHANT_ARMOR, 50},
    { TV_SCROLL, SV_SCROLL_DETECT_ITEM, 50},
    { TV_SCROLL, SV_SCROLL_DETECT_GOLD, 50},
    { TV_SCROLL, SV_SCROLL_LIGHT, 50},
    { TV_SCROLL, SV_SCROLL_MAPPING, 50},
    { TV_SCROLL, SV_SCROLL_MONSTER_CONFUSION, 50},
    { TV_SCROLL, SV_SCROLL_RECHARGING, 50},
    { TV_SCROLL, SV_SCROLL_BLESSING, 50},
    { TV_SCROLL, SV_SCROLL_HOLY_PRAYER, 50},
    { TV_SCROLL, SV_SCROLL_PROTECTION_FROM_EVIL, 50},
    { TV_SCROLL, SV_SCROLL_DETECT_INVIS, 50},
    { TV_SCROLL, SV_SCROLL_DISPEL_UNDEAD, 50},
    { 0, 0, 0}
};

static int _gamble_shop_roll(const _gamble_shop_t *choices)
{
    int tot = 0, roll;
    int i = 0;

    for (i = 0; ; i++)
    {
        const _gamble_shop_t *entry = choices + i;
        if (!entry->prob)
            break;
        tot += entry->prob;
    }

    roll = randint1(tot);

    for (i = 0; ; i++)
    {
        const _gamble_shop_t *entry = choices + i;
        if (!entry->prob)
            break;

        roll -= entry->prob;
        if (roll <= 0)
            return i;
    }

    return -1;
}

static bool _gamble_shop_aux(object_type *o_ptr)
{
    char buf[MAX_NLEN];
    int slot, auto_pick_idx;

    obj_identify_fully(o_ptr);
    stats_on_identify(o_ptr);
    object_desc(buf, o_ptr, OD_COLOR_CODED);

    clear_bldg(5, 10);
    
    c_put_str(TERM_YELLOW, "You Win:", 5, 0);
    put_str(buf, 5, 9);

    auto_pick_idx = is_autopick(o_ptr);
    if (auto_pick_idx >= 0)
    {
        if (autopick_list[auto_pick_idx].action & DO_AUTODESTROY)
        {
            msg_format("You destroy %s.", buf);
            return TRUE;
        }
    }

    if (!inven_carry_okay(o_ptr))
    {
        msg_print("You cannot carry that many different items.");
        /* Charge the player anyway. Otherwise, they can get whatever object
           they want by playing games with their inventory! */
        return TRUE;
    }
    slot = inven_carry(o_ptr);
    object_desc(buf, &inventory[slot], OD_COLOR_CODED);
    msg_format("You have %s (%c).", buf, index_to_label(slot));
    handle_stuff();
    return TRUE;
}

static bool _gamble_shop(const _gamble_shop_t *choices)
{
    object_type forge = {0};
    int k_idx;
    int choice = _gamble_shop_roll(choices);

    if (choice < 0)
    {
        msg_print("Oops! Something went wrong.");
        return FALSE;
    }

    k_idx = lookup_kind(choices[choice].tval, choices[choice].sval);
    object_prep(&forge, k_idx);

    return _gamble_shop_aux(&forge);
}

static bool _gamble_shop_object(object_p pred)
{
    object_type forge = {0};
    int lvl = 40 + randint0(60);
    int k_idx;
    
    for (;;)
    {
        k_idx = get_obj_num(lvl);
        object_prep(&forge, k_idx);
        if (pred && !pred(&forge))
            continue;
        apply_magic(&forge, lvl, AM_GOOD);
        switch (forge.tval)
        {
            case TV_SPIKE:
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:
            {
                if (!forge.name1 && !forge.name3)
                    forge.number = (byte)damroll(6, 7);
            }
        }
        break;
    }

    return _gamble_shop_aux(&forge);
}

static bool _gamble_shop_device(int tval)
{
    object_type forge = {0};
    int lvl = 20;
    int k_idx;

    while (one_in_(lvl/10))
        lvl += 20;
	
    for (;;)
    {
        k_idx = lookup_kind(tval, SV_ANY);
        object_prep(&forge, k_idx);
        if (device_init(&forge, lvl, 0))
            break;
    }

    return _gamble_shop_aux(&forge);
}

static bool _gamble_shop_artifact(void)
{
    object_type forge = {0};
    int lvl = 70 + randint0(30);
    int k_idx;
    
    for (;;)
    {
        k_idx = get_obj_num(lvl);
        object_prep(&forge, k_idx);
        if (!object_is_weapon(&forge) && !object_is_armour(&forge))
            continue;
        apply_magic(&forge, lvl, AM_GOOD | AM_GREAT | AM_SPECIAL);
        if (!forge.art_name)
            continue;

        break;
    }

    return _gamble_shop_aux(&forge);
}

static bool _reforge_artifact(void)
{
    int src_idx, dest_idx, cost;
    char o_name[MAX_NLEN];
    char buf[255];
    object_type *src, *dest;
    int f = MIN(200, p_ptr->fame);
	if (coffeebreak_mode) f *= 2;
    int src_max_power = f*250 + f*f*3;
    int dest_max_power = 0;

    if (p_ptr->prace == RACE_MON_SWORD || p_ptr->prace == RACE_MON_RING)
    {
        msg_print("Go enchant yourself!");
        return FALSE;
    }

    item_tester_hook = object_is_artifact;
    sprintf(buf, "Use what artifact for reforging (Max Power = %d)? ", src_max_power);
    if (!get_item(&src_idx, buf, "You have no artifacts to reforge.", USE_INVEN | SHOW_VALUE))
        return FALSE;

    src = &inventory[src_idx];
    if (!object_is_artifact(src)) /* paranoia */
    {
        msg_print("You must choose an artifact for reforging.");
        return FALSE;
    }
    if (obj_value_real(src) > src_max_power)
    {
        msg_print("You are not famous enough to reforge that item.");
        return FALSE;
    }

    cost = obj_value_real(src);
    
    dest_max_power = cost / 2;
    if (dest_max_power < 1000) /* Reforging won't try to power match weak stuff ... */
        dest_max_power = 1000;
    
    cost *= 10;
    cost -= cost % 1000;

    if (cost < 100000)
        cost = 100000;
    if (cost > 25000000)
        cost = 25000000;

    msg_format("Reforging will cost you %d gold.", cost);
    if (p_ptr->au < cost)
    {
        msg_print("You do not have that much.");
        return FALSE;
    }

    object_desc(o_name, src, OD_NAME_ONLY | OD_COLOR_CODED);
    if (!get_check(format("Really use %s? (It will be destroyed!) ", o_name))) 
        return FALSE;

    sprintf(buf, "Reforge which object (Max Power = %d)? ", dest_max_power);
    item_tester_hook = item_tester_hook_nameless_weapon_armour;
    if (!get_item(&dest_idx, buf, "You have nothing to reforge.", USE_EQUIP | USE_INVEN | SHOW_VALUE))
        return FALSE;

    dest = &inventory[dest_idx];

    if (dest->number > 1)
    {
        msg_print("Don't be greedy! You may only reforge a single object.");
        return FALSE;
    }

    if (object_is_artifact(dest))
    {
        msg_print("This item is already an artifact!");
        return FALSE;
    }

    if (have_flag(dest->flags, OF_NO_REMOVE))
    {
        msg_print("You cannot be reforged!");
        return FALSE;
    }

    if (object_is_ego(dest))
    {
        msg_print("This item is already an ego item!");
        return FALSE;
    }

    if (!equip_first_slot(dest))
    {
        msg_print("This item cannot be reforged.");
        return FALSE;
    }

    if (obj_value_real(dest) > dest_max_power)
    {
        msg_print("This item is too powerful for the source artifact you have chosen.");
        return FALSE;
    }

    if (!reforge_artifact(src, dest, p_ptr->fame))
    {
        msg_print("The reforging failed!");
        return FALSE;
    }

    inven_item_increase(src_idx, -1);
    inven_item_describe(src_idx);
    p_ptr->au -= cost;
    stats_on_gold_services(cost);

    msg_print("After several hours, you are presented your new artifact...");

    obj_identify_fully(dest);

    p_ptr->update |= PU_BONUS;
    p_ptr->window |= (PW_INVEN | PW_EQUIP);
    handle_stuff();

    obj_display(dest);

    inven_item_optimize(src_idx);

    return TRUE;
}

typedef struct _enchant_choice_s { int amt; int cost; } _enchant_choice_t;
static void _enchant_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    _enchant_choice_t *ptr = (_enchant_choice_t *)cookie;
    ptr += which;
    switch (cmd)
    {
    case MENU_TEXT:
    {
        char tmp[255];
        if (ptr->cost >= 10000)
            big_num_display(ptr->cost, tmp);
        else
            sprintf(tmp, "%d", ptr->cost);

        var_set_string(res, format("+%2d %9.9s", ptr->amt, tmp));
        break;
    }
    case MENU_COLOR:
        if (ptr->cost > p_ptr->au)
        {
            var_set_int(res, TERM_L_DARK);
            break;
        }
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static bool _library_can_read(int sval){
	/* For berserkers, so they can make some use out of acquirements and whatnot. */
	switch (sval){
	case SV_SCROLL_ACQUIREMENT:
	case SV_SCROLL_STAR_ACQUIREMENT:
	case SV_SCROLL_CRAFTING:
	case SV_SCROLL_CURSE_ARMOR:
	case SV_SCROLL_CURSE_WEAPON:
	case SV_SCROLL_ENCHANT_ARMOR:
	case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
	case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
	case SV_SCROLL_MUNDANITY:
	case SV_SCROLL_IDENTIFY:
	case SV_SCROLL_STAR_IDENTIFY:
	case SV_SCROLL_STAR_REMOVE_CURSE:
	case SV_SCROLL_REMOVE_CURSE:
	case SV_SCROLL_RECHARGING:
	case SV_SCROLL_ARTIFACT:
	case SV_SCROLL_RESET_RECALL:
	case SV_SCROLL_WORD_OF_RECALL:
		return TRUE;
	}
	return FALSE;
}

static bool _read_scroll_for_player(void){

	int         item, cost = 0;
	bool        okay = FALSE;
	object_type *o_ptr;
	cptr        q, s;
	char        tmp_str[MAX_NLEN];
	int         store_factor = store_calc_price_factor(100);


	clear_bldg(4, 18);

	q = "Which scroll shall I read for you? ";
	s = "You have no scrolls. ";
	if (!get_item(&item, q, s, USE_INVEN )) return (FALSE);
	o_ptr = &inventory[item];

	if (cost == 0) cost = store_calc_sell_price(obj_value_real(o_ptr) / 2, store_factor);

	/* Streamline. Nothing is more fun then enchanting Twilight (-40,-60)->(+10, +10), I
	admit. But other players might not share my love of carpal tunnel syndrome! */
	{
		if (!_library_can_read(o_ptr->sval))
		{
			object_desc(tmp_str, o_ptr, OD_COLOR_CODED | OD_NO_PLURAL | OD_OMIT_PREFIX);
			msg_format("I will not read %^s for you. ", tmp_str);
			return FALSE;
		}
	}

	char prompt[255];
	object_desc(tmp_str, o_ptr, OD_COLOR_CODED | OD_NO_PLURAL | OD_OMIT_PREFIX);
	sprintf(prompt, "Reading %s will cost %d gold. y/n? ", tmp_str, cost);
	if (msg_prompt(prompt, "ny", PROMPT_YES_NO) == 'n') return FALSE;

	/* Check if the player has enough money */
	if (p_ptr->au < cost)
	{
		msg_format("You don't have enough gold.");
		return FALSE;
	}

	okay = device_use(o_ptr, 0);

	if (!okay)
	{
		if (flush_failure) flush();
		msg_print("Scroll wasn't read.");
		return (FALSE);
	}
	else
	{
		
		if (item >= 0)
		{
			inven_item_increase(item, -1);
			inven_item_describe(item);
			inven_item_optimize(item);
		}
		
		p_ptr->au -= cost;
		stats_on_gold_services(cost);
	
		return (TRUE);
	}
	
}

static bool enchant_item(int cost, int to_hit, int to_dam, int to_ac, bool is_guild)
{
    int         i, item;
    bool        okay = FALSE;
    object_type *o_ptr;
    cptr        q, s;
    int         maxenchant;
    char        tmp_str[MAX_NLEN];
    int         store_factor = store_calc_price_factor(100);

    if (cost == 0)
        cost = store_calc_sell_price(1500, store_factor);

    if (p_ptr->prace == RACE_MON_SWORD)
    {
        msg_print("Go enchant yourself!");
        return FALSE;
    }

    clear_bldg(4, 18);

    /* Which Item? Client sets item_tester_hook! */
    item_tester_no_ryoute = TRUE;
    q = "Improve which item? ";
    s = "You have nothing to improve.";
    if (!get_item(&item, q, s, (USE_INVEN | USE_EQUIP))) return (FALSE);
    o_ptr = &inventory[item];

    if (o_ptr->tval == TV_ARROW || o_ptr->tval == TV_BOLT || o_ptr->tval == TV_SHOT)
        maxenchant = (p_ptr->lev / 5);
    else if (is_guild)
        maxenchant = 5 + (p_ptr->lev / 5);
    else
        maxenchant = 2 + (p_ptr->lev / 5);

    /* Streamline. Nothing is more fun then enchanting Twilight (-40,-60)->(+10, +10), I 
       admit. But other players might not share my love of carpal tunnel syndrome! */
    {
        int idx = -1;
        int old_cost;
        int unit_cost_sum = 0;
        _enchant_choice_t choices[25];
        object_type copy = {0};
        menu_t menu = { "Enchant How Much?", NULL, 
                        "Amt      Cost", _enchant_menu_fn,
                        choices, 25 };

        object_copy(&copy, o_ptr);
        copy.curse_flags = 0;
        remove_flag(copy.flags, OF_AGGRAVATE);
        remove_flag(copy.flags, OF_NO_TELE);
        remove_flag(copy.flags, OF_NO_MAGIC);
        remove_flag(copy.flags, OF_DRAIN_EXP);
        remove_flag(copy.flags, OF_TY_CURSE);
        old_cost = new_object_cost(&copy, COST_REAL);
                
        for (i = 0; i < 25; i++) /* TODO: Option for max. But +25 a pop is enough perhaps? */
        {
            bool ok = FALSE;
            int  v = 0, m = 5;
            
            choices[i].amt = i+1;

            if (to_hit && copy.to_h < maxenchant) {copy.to_h++; ok = TRUE; v = MAX(v, copy.to_h);}
            if (to_dam && copy.to_d < maxenchant) {copy.to_d++; ok = TRUE; v = MAX(v, copy.to_d);}
            if (to_ac && copy.to_a < maxenchant) {copy.to_a++; ok = TRUE; v = MAX(v, copy.to_a);}

            if (v > 10)
            {
                if (to_ac)
                {
                    int j;
                    for (j = 10; j < v; j++)
                        m = m * 5 / 3;
                }
                else
                    m += v - 10;
            }

            if (object_is_artifact(o_ptr))
                m *= 3;

            if (!ok) break;

            if (o_ptr->tval == TV_ARROW || o_ptr->tval == TV_BOLT || o_ptr->tval == TV_SHOT)
            {
                choices[i].cost = (i+1)*cost*o_ptr->number;
            }
            else
            {
                int new_cost = new_object_cost(&copy, COST_REAL);
                int unit_cost_add = new_cost - old_cost;
                int min_cost = (i+1)*cost;
                int unit_cost;
                old_cost = new_cost;
                
                unit_cost_add *= m;
                unit_cost_sum += unit_cost_add;
                unit_cost = unit_cost_sum;

                unit_cost = store_calc_sell_price(unit_cost, store_factor);

                if (unit_cost < min_cost)
                    unit_cost = min_cost;

                if (is_guild)
                    unit_cost = (unit_cost + 1)/2;

                if (unit_cost >= 10000)
                    choices[i].cost = big_num_round(unit_cost, 3);
                else
                    choices[i].cost = unit_cost;
            }
        }
        if (!i)
        {
            object_desc(tmp_str, o_ptr, OD_COLOR_CODED);
            msg_format("%^s can not be further improved.", tmp_str);
            return FALSE;
        }

        menu.count = i;
        idx = menu_choose(&menu);
        if (idx < 0) return FALSE;
        
        if (to_hit) to_hit = choices[idx].amt;
        if (to_dam) to_dam = choices[idx].amt;
        if (to_ac) to_ac = choices[idx].amt;
        cost = choices[idx].cost;
    }

    /* Check if the player has enough money */
    if (p_ptr->au < cost)
    {
        object_desc(tmp_str, o_ptr, OD_NAME_ONLY | OD_COLOR_CODED);
        msg_format("You do not have the gold to improve %s!", tmp_str);
        return FALSE;
    }

    /* Enchant to hit */
    for (i = 0; i < to_hit; i++)
    {
        if (o_ptr->to_h < maxenchant)
        {
            if (enchant(o_ptr, 1, (ENCH_TOHIT | ENCH_FORCE)))
                okay = TRUE;
        }
    }

    /* Enchant to damage */
    for (i = 0; i < to_dam; i++)
    {
        if (o_ptr->to_d < maxenchant)
        {
            if (enchant(o_ptr, 1, (ENCH_TODAM | ENCH_FORCE)))
                okay = TRUE;
        }
    }

    /* Enchant to AC */
    for (i = 0; i < to_ac; i++)
    {
        if (o_ptr->to_a < maxenchant)
        {
            if (enchant(o_ptr, 1, (ENCH_TOAC | ENCH_FORCE)))
                okay = TRUE;
        }
    }

    if (!okay)
    {
        if (flush_failure) flush();
        msg_print("The improvement failed.");
        return (FALSE);
    }
    else
    {
        object_desc(tmp_str, o_ptr, OD_NAME_AND_ENCHANT | OD_COLOR_CODED);
        msg_format("Improved into %s for %d gold.", tmp_str, cost);

        p_ptr->au -= cost;
        stats_on_gold_services(cost);
        if (equip_is_valid_slot(item)) android_calc_exp();
        return (TRUE);
    }
}

bool tele_town(void)
{
    int i, x, y;
    int num = 0;

    if (dun_level)
    {
        msg_print("This spell can only be used on the surface!");
        return FALSE;
    }

    if (p_ptr->inside_arena || p_ptr->inside_battle)
    {
        msg_print("This spell can only be used outside!");
        return FALSE;
    }

    screen_save();
    clear_bldg(4, 10);

    for (i=1;i<max_towns;i++)
    {
        char buf[80];

        if ((i == NO_TOWN) || 
            (i == SECRET_TOWN) || 
            (i == p_ptr->town_num) || 
            (!(p_ptr->visit & (1L << (i-1))) && !p_ptr->wizard)) continue;

        sprintf(buf,"%c) %-20s", I2A(i-1), town[i].name);
        prt(buf, 5+i, 5);
        num++;
    }

    if (!num)
    {
        msg_print("You have not yet visited any town.");
        msg_print(NULL);
        screen_load();
        return FALSE;
    }

    prt("Which town you go: ", 0, 0);
    while(1)
    {
        i = inkey();

        if (i == ESCAPE)
        {
            screen_load();
            return FALSE;
        }
        else if ((i < 'a') || (i > ('a'+max_towns-2))) continue;
        else if (((i-'a'+1) == p_ptr->town_num) || ((i-'a'+1) == NO_TOWN) || ((i-'a'+1) == SECRET_TOWN) || (!(p_ptr->visit & (1L << (i-'a'))) && !p_ptr->wizard)) continue;
        break;
    }

    for (y = 0; y < max_wild_y; y++)
    {
        for (x = 0; x < max_wild_x; x++)
        {
            if(wilderness[y][x].town == (i-'a'+1))
            {
                p_ptr->wilderness_y = y;
                p_ptr->wilderness_x = x;

                p_ptr->wilderness_dx = 0;
                p_ptr->wilderness_dy = 0;
            }
        }
    }

    p_ptr->leaving = TRUE;
    leave_bldg = TRUE;
    p_ptr->teleport_town = TRUE;
    screen_load();
    return TRUE;
}


/*
 *  research_mon
 *  -KMW-
 */
static bool research_mon(void)
{
    int i, n, r_idx;
    char sym, query;
    char buf[128];

    bool notpicked;

    bool recall = FALSE;

    u16b why = 0;

    u16b    *who;

    /* XTRA HACK WHATSEARCH */
    bool    all = FALSE;
    bool    uniq = FALSE;
    bool    norm = FALSE;
    char temp[80] = "";

    /* XTRA HACK REMEMBER_IDX */
    static int old_sym = '\0';
    static int old_i = 0;


    /* Save the screen */
    screen_save();

    /* Get a character, or abort */
    if (!get_com("Enter character to be identified(^A:All,^U:Uniqs,^N:Non uniqs,^M:Name): ", &sym, FALSE))

    {
        /* Restore */
        screen_load();

        return (FALSE);
    }

    /* Find that character info, and describe it */
    for (i = 0; ident_info[i]; ++i)
    {
        if (sym == ident_info[i][0]) break;
    }

        /* XTRA HACK WHATSEARCH */
    if (sym == KTRL('A'))
    {
        all = TRUE;
        strcpy(buf, "Full monster list.");
    }
    else if (sym == KTRL('U'))
    {
        all = uniq = TRUE;
        strcpy(buf, "Unique monster list.");
    }
    else if (sym == KTRL('N'))
    {
        all = norm = TRUE;
        strcpy(buf, "Non-unique monster list.");
    }
    else if (sym == KTRL('M'))
    {
        all = TRUE;
        if (!get_string("Enter name:",temp, 70))
        {
            temp[0]=0;

            /* Restore */
            screen_load();

            return FALSE;
        }
        sprintf(buf, "Monsters with a name \"%s\"",temp);
    }
    else if (ident_info[i])
    {
        sprintf(buf, "%c - %s.", sym, ident_info[i] + 2);
    }
    else
    {
        sprintf(buf, "%c - %s.", sym, "Unknown Symbol");

    }

    /* Display the result */
    prt(buf, 16, 10);


    /* Allocate the "who" array */
    C_MAKE(who, max_r_idx, u16b);

    /* Collect matching monsters */
    for (n = 0, i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /* Empty monster */
        if (!r_ptr->name) continue;

        /* XTRA HACK WHATSEARCH */
        /* Require non-unique monsters if needed */
        if (norm && (r_ptr->flags1 & (RF1_UNIQUE))) continue;

        /* Require unique monsters if needed */
        if (uniq && !(r_ptr->flags1 & (RF1_UNIQUE))) continue;

        if (temp[0])
        {
            int xx;
            char temp2[80];

            for (xx = 0; temp[xx] && xx < 80; xx++)
            {
                if (isupper(temp[xx])) temp[xx] = tolower(temp[xx]);
            }
  
            strcpy(temp2, r_name + r_ptr->name);
            for (xx = 0; temp2[xx] && xx < 80; xx++)
                if (isupper(temp2[xx])) temp2[xx] = tolower(temp2[xx]);

            if (my_strstr(temp2, temp))
                who[n++] = i;
        }
        else if (all || (r_ptr->d_char == sym)) who[n++] = i;
    }

    /* Nothing to recall */
    if (!n)
    {
        /* Free the "who" array */
        C_KILL(who, max_r_idx, u16b);

        /* Restore */
        screen_load();

        return (FALSE);
    }

    /* Sort by level */
    why = 2;
    query = 'y';

    /* Sort if needed */
    if (why)
    {
        /* Select the sort method */
        ang_sort_comp = ang_sort_comp_hook;
        ang_sort_swap = ang_sort_swap_hook;

        /* Sort the array */
        ang_sort(who, &why, n);
    }


    /* Start at the end */
    /* XTRA HACK REMEMBER_IDX */
    if (old_sym == sym && old_i < n) i = old_i;
    else i = n - 1;

    notpicked = TRUE;

    /* Scan the monster memory */
    while (notpicked)
    {
        /* Extract a race */
        r_idx = who[i];

        /* Hack -- Begin the prompt */
        roff_top(r_idx);

        /* Hack -- Complete the prompt */
        Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC, space to continue]");


        /* Interact */
        while (1)
        {
            /* Recall */
            if (recall)
            {
                /*** Recall on screen ***/

                /* Mark Monster as Seen for Monster Knowledge Screens */
                if (!r_info[r_idx].r_sights)
                    r_info[r_idx].r_sights = 1;

                /* Get maximal info about this monster */
                lore_do_probe(r_idx);

                /* Save this monster ID */
                monster_race_track(r_idx);

                /* Hack -- Handle stuff */
                handle_stuff();

                /* know every thing mode */
                mon_display(&r_info[r_idx]);
                notpicked = FALSE;

                /* XTRA HACK REMEMBER_IDX */
                old_sym = sym;
                old_i = i;
            }

            /* Command */
            query = inkey();

            /* Normal commands */
            if (query != 'r') break;

            /* Toggle recall */
            recall = !recall;
        }

        /* Stop scanning */
        if (query == ESCAPE) break;

        /* Move to "prev" monster */
        if (query == '-')
        {
            if (++i == n)
            {
                i = 0;
                if (!expand_list) break;
            }
        }

        /* Move to "next" monster */
        else
        {
            if (i-- == 0)
            {
                i = n - 1;
                if (!expand_list) break;
            }
        }
    }


    /* Re-display the identity */
    /* prt(buf, 5, 5);*/

    /* Free the "who" array */
    C_KILL(who, max_r_idx, u16b);

    /* Restore */
    screen_load();

    return (!notpicked);
}

/*
 * Execute a building command
 */
static void bldg_process_command(building_type *bldg, int i)
{
    int bact = bldg->actions[i];
    int bcost;
    bool paid = FALSE;
    int amt;
    bool is_guild = FALSE;

    /* Flush messages XXX XXX XXX */
    msg_line_clear();

    if (is_owner(bldg))
    {
        bcost = bldg->member_costs[i];
        is_guild = TRUE;
    }
    else
        bcost = bldg->other_costs[i];

    /* Adjst price for race, charisma and fame */
    bcost = store_calc_sell_price(bcost, store_calc_price_factor(100));

    /* action restrictions */
    if (((bldg->action_restr[i] == 1) && !is_member(bldg)) ||
        ((bldg->action_restr[i] == 2) && !is_owner(bldg)))
    {
        msg_print("You have no right to choose that!");
        return;
    }

    if (bcost > p_ptr->au)
    {
        msg_print("You do not have the gold!");
        return;
    }

    switch (bact)
    {
    case BACT_NOTHING:
        /* Do nothing */
        break;
    case BACT_RESEARCH_ITEM:
        paid = identify_fully(NULL);
        break;
    case BACT_TOWN_HISTORY:
        town_history();
        break;
    case BACT_RACE_LEGENDS:
        race_legends();
        break;
    case BACT_QUEST:
        castle_quest();
        break;
    case BACT_KING_LEGENDS:
    case BACT_ARENA_LEGENDS:
    case BACT_LEGENDS:
        show_highclass();
        break;
    case BACT_POSTER:
    case BACT_ARENA_RULES:
    case BACT_ARENA:
        arena_comm(bact);
        break;
    case BACT_IN_BETWEEN:
    case BACT_CRAPS:
    case BACT_SPIN_WHEEL:
    case BACT_DICE_SLOTS:
    case BACT_GAMBLE_RULES:
    case BACT_POKER:
        gamble_comm(bact);
        break;
    case BACT_REST:
    case BACT_RUMORS:
    case BACT_FOOD:
        paid = inn_comm(bact);
        break;
    case BACT_RESEARCH_MONSTER:
        paid = research_mon();
        break;
    case BACT_COMPARE_WEAPONS:
    /*    paid = compare_weapons(); */
        break;
    case BACT_ENCHANT_WEAPON:
        item_tester_hook = object_allow_enchant_melee_weapon;
        enchant_item(bcost, 1, 1, 0, is_guild);
        break;
    case BACT_ENCHANT_ARMOR:
        item_tester_hook = object_is_armour;
        enchant_item(bcost, 0, 0, 1, is_guild);
        break;
    case BACT_RECHARGE:
        msg_print("My apologies, but that service is no longer available!");
        break;
    case BACT_RECHARGE_ALL:
        msg_print("My apologies, but that service is no longer available!");
        break;
    case BACT_IDENTS: /* needs work */
        if (!get_check("Do you pay for identify all your possession? ")) break;
        identify_pack();
        msg_print("Your possessions have been identified.");

        paid = TRUE;
        break;
    case BACT_IDENT_ONE: /* needs work */
        paid = ident_spell(NULL);
        break;
    case BACT_LEARN:
        do_cmd_study();
        break;
    case BACT_HEALING: /* needs work */
        hp_player(200);
        set_poisoned(0, TRUE);
        set_blind(0, TRUE);
        set_confused(0, TRUE);
        set_cut(0, TRUE);
        set_stun(0, TRUE);
		restore_level(); // restore levels too.

        if (p_ptr->riding)
        {
            monster_type *m_ptr = &m_list[p_ptr->riding];
            int           amt = 500;

            if (m_ptr->hp < 30000) m_ptr->hp += amt;
            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
            p_ptr->redraw |= PR_HEALTH_BARS;
        }

        paid = TRUE;
        break;
    case BACT_RESTORE: /* needs work */
        if (do_res_stat(A_STR)) paid = TRUE;
        if (do_res_stat(A_INT)) paid = TRUE;
        if (do_res_stat(A_WIS)) paid = TRUE;
        if (do_res_stat(A_DEX)) paid = TRUE;
        if (do_res_stat(A_CON)) paid = TRUE;
        if (do_res_stat(A_CHR)) paid = TRUE;
        break;
    case BACT_ENCHANT_ARROWS:
        item_tester_hook = item_tester_hook_ammo;
        enchant_item(bcost, 1, 1, 0, is_guild);
        break;
    case BACT_ENCHANT_BOW:
        item_tester_tval = TV_BOW;
        enchant_item(bcost, 1, 1, 0, is_guild);
        break;
    case BACT_RECALL:
        if (recall_player(1)) paid = TRUE;
        break;
    case BACT_TELEPORT_LEVEL:
    {
        int select_dungeon;
        int max_depth;

        clear_bldg(4, 20);
        select_dungeon = choose_dungeon("teleport", 4, 0);
        show_building(bldg);
        if (!select_dungeon) return;

        max_depth = d_info[select_dungeon].maxdepth;

        /* Limit depth in Angband */
        if (select_dungeon == DUNGEON_ANGBAND)
        {
            if (quest[QUEST_OBERON].status != QUEST_STATUS_FINISHED) max_depth = 98;
            else if(quest[QUEST_SERPENT].status != QUEST_STATUS_FINISHED) max_depth = 99;
        }

        amt = get_quantity(format("Teleport to which level of %s? ", d_name + d_info[select_dungeon].name), max_depth);
        if (amt > 0)
        {
            p_ptr->word_recall = 1;
            p_ptr->recall_dungeon = select_dungeon;
            max_dlv[p_ptr->recall_dungeon] = ((amt > d_info[select_dungeon].maxdepth) ? d_info[select_dungeon].maxdepth : ((amt < d_info[select_dungeon].mindepth) ? d_info[select_dungeon].mindepth : amt));
            msg_print("The air about you becomes charged...");

            paid = TRUE;
            p_ptr->redraw |= (PR_STATUS);
        }
        break;
    }
    case BACT_LOSE_MUTATION:
        if (mut_count(mut_unlocked_pred))
        {
            paid = mut_lose_random(NULL);
        }
        else
        {
            msg_print("You have no mutations that I can cure.");
        }
        break;
    case BACT_BATTLE:
        kakutoujou();
        break;
    case BACT_TSUCHINOKO:
        tsuchinoko();
        break;
    case BACT_KUBI:
        shoukinkubi();
        break;
    case BACT_TARGET:
        today_target();
        break;
    case BACT_KANKIN:
        kankin();
        break;
    case BACT_HEIKOUKA:
        msg_print("You received an equalization ritual.");
        virtue_init();
        paid = TRUE;
        break;
    case BACT_TELE_TOWN:
        paid = tele_town();
        break;
    case BACT_EVAL_AC:
        paid = eval_ac(p_ptr->dis_ac + p_ptr->dis_to_a);
        break;
    case BACT_GAMBLE_SHOP_POTION:
        paid = _gamble_shop(_gamble_shop_potions);
        break;
    case BACT_GAMBLE_SHOP_SCROLL:
        paid = _gamble_shop(_gamble_shop_scrolls);
        break;
    case BACT_GAMBLE_SHOP_STAFF:
        paid = _gamble_shop_device(TV_STAFF);
        break;
    case BACT_GAMBLE_SHOP_WAND:
        paid = _gamble_shop_device(TV_WAND);
        break;
    case BACT_GAMBLE_SHOP_ROD:
        paid = _gamble_shop_device(TV_ROD);
        break;
    case BACT_GAMBLE_SHOP_ARMOR:
        paid = _gamble_shop_object(object_is_armour);
        break;
    case BACT_GAMBLE_SHOP_WEAPON:
        paid = _gamble_shop_object(object_is_weapon);
        break;
    case BACT_GAMBLE_SHOP_ARTIFACT:
        paid = _gamble_shop_artifact();
        break;
    case BACT_REFORGE_ARTIFACT:
        paid = _reforge_artifact();
        break;
    case BACT_CHANGE_NAME:
        if (py_get_name())
        {
            process_player_name(FALSE);
            paid = TRUE;
        }
        break;
	case BACT_READ_SCROLL:
			item_tester_hook = object_is_scroll;
			_read_scroll_for_player();
			break;
		
    case BACT_REPUTATION:
        if (p_ptr->fame <= 0)
            cmsg_print(TERM_WHITE, "Who the hell are you?");
        else if (p_ptr->fame < 20)
            cmsg_print(TERM_WHITE, "I've never even heard of you!");
        else if (p_ptr->fame < 40)
            cmsg_print(TERM_L_UMBER, "Hmmm ... You've done a few minor notable deeds, but hardly anything worth bragging about!");
        else if (p_ptr->fame < 60)
            cmsg_print(TERM_YELLOW, "Yes, I've heard of you. The townfolk are talking!");
        else if (p_ptr->fame < 80)
            cmsg_print(TERM_ORANGE, "Ah, good sir. 'Tis an honor to see you again!");
        else if (p_ptr->fame < 100)
            cmsg_print(TERM_L_RED, "You are a true hero!");
        else if (p_ptr->fame < 150)
            cmsg_print(TERM_RED, "You are the stuff of legends!");
        else
            cmsg_print(TERM_VIOLET, "The bards doth sing of ye: Heroic ballads both far 'n wide!");
        paid = TRUE;
        break;
    }

    if (paid)
    {
        p_ptr->au -= bcost;
        stats_on_gold_services(bcost);
        if (prace_is_(RACE_MON_LEPRECHAUN))
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
    }

}


/*
 * Enter quest level
 */
void do_cmd_quest(void)
{
    energy_use = 100;

    if (!cave_have_flag_bold(py, px, FF_QUEST_ENTER))
    {
        msg_print("You see no quest level here.");
        return;
    }
    else
    {
        msg_print("There is an entry of a quest.");
        if (!get_check("Do you enter? ")) return;

        /* Player enters a new quest */
        p_ptr->oldpy = 0;
        p_ptr->oldpx = 0;

        leave_quest_check();

        if (quest[p_ptr->inside_quest].type != QUEST_TYPE_RANDOM) dun_level = 1;
        p_ptr->inside_quest = cave[py][px].special;

        p_ptr->leaving = TRUE;
    }
}


/*
 * Do building commands
 */
void do_cmd_bldg(void)
{
    int             i, which;
    char            command;
    bool            validcmd;
    building_type   *bldg;
    rect_t          msg_display_rect;


    energy_use = 100;

    if (!cave_have_flag_bold(py, px, FF_BLDG))
    {
        msg_print("You see no building here.");

        return;
    }

    which = f_info[cave[py][px].feat].subtype;

    bldg = &building[which];

    /* Don't re-init the wilderness */
    reinit_wilderness = FALSE;

    if ((which == 2) && (p_ptr->arena_number < 0))
    {
        msg_print("'There's no place here for a LOSER like you!'");
        return;
    }
    else if ((which == 2) && p_ptr->inside_arena)
    {
        if (!p_ptr->exit_bldg)
        {
            prt("The gates are closed. The monster awaits!", 0, 0);
        }
        else
        {
            /* Don't save the arena as saved floor */
            prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_NO_RETURN);

            p_ptr->inside_arena = FALSE;
            p_ptr->leaving = TRUE;

            /* Re-enter the arena */
            command_new = SPECIAL_KEY_BUILDING;

            /* No energy needed to re-enter the arena */
            energy_use = 0;
        }

        return;
    }
    else if (p_ptr->inside_battle)
    {
        /* Don't save the arena as saved floor */
        prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_NO_RETURN);

        p_ptr->leaving = TRUE;
        p_ptr->inside_battle = FALSE;

        /* Re-enter the monster arena */
        command_new = SPECIAL_KEY_BUILDING;

        /* No energy needed to re-enter the arena */
        energy_use = 0;

        return;
    }
    else
    {
        p_ptr->oldpy = py;
        p_ptr->oldpx = px;
    }

    /* Forget the lite */
    forget_lite();

    /* Forget the view */
    forget_view();

    /* Hack -- Increase "icky" depth */
    character_icky++;

    command_arg = 0;
    command_rep = 0;
    command_new = 0;

    leave_bldg = FALSE;
    show_building(bldg);

    msg_display_rect = rect_create(0, 0, 80, 3);
    msg_line_init(&msg_display_rect);
    store_hack = TRUE;

    while (!leave_bldg)
    {
        validcmd = FALSE;

        building_prt_gold();

        command = inkey();

        if (command == ESCAPE)
        {
            leave_bldg = TRUE;
            p_ptr->inside_arena = FALSE;
            p_ptr->inside_battle = FALSE;
            break;
        }

        for (i = 0; i < 8; i++)
        {
            if (bldg->letters[i])
            {
                if (bldg->letters[i] == command)
                {
                    validcmd = TRUE;
                    break;
                }
            }
        }

        if (validcmd)
        {
            bldg_process_command(bldg, i);
        }

        /* Notice stuff */
        notice_stuff();

        /* Handle stuff */
        handle_stuff();
    }

    store_hack = FALSE;
    msg_line_init(NULL);

    /* Reinit wilderness to activate quests ... */
    if (reinit_wilderness)
    {
        p_ptr->leaving = TRUE;
    }

    /* Hack -- Decrease "icky" depth */
    character_icky--;

    /* Clear the screen */
    Term_clear();

    /* Update the visuals */
    p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_BONUS | PU_LITE | PU_MON_LITE);

    /* Redraw entire screen */
    p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_EQUIPPY | PR_MAP);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
}


/* Array of places to find an inscription */
static cptr find_quest[] =
{
    "You find the following inscription in the floor",

    "You see a message inscribed in the wall",

    "There is a sign saying",

    "Something is written on the staircase",

    "You find a scroll with the following message",

};


/*
 * Discover quest
 */
void quest_discovery(int q_idx)
{
    quest_type      *q_ptr = &quest[q_idx];
    monster_race    *r_ptr = &r_info[q_ptr->r_idx];
    int             q_num = q_ptr->max_num;
    char            name[80];

    /* No quest index */
    if (!q_idx) return;

    strcpy(name, (r_name + r_ptr->name));

    msg_print(find_quest[rand_range(0, 4)]);

    if (q_num == 1)
    {
        /* Unique */

        /* Hack -- "unique" monsters must be "unique" */
        if ((r_ptr->flags1 & RF1_UNIQUE) &&
            (0 == r_ptr->max_num))
        {
            msg_print("It seems that this level was protected by someone before...");
            /* The unique is already dead */
            quest[q_idx].status = QUEST_STATUS_FINISHED;
        }
        else
        {
            cmsg_format(TERM_VIOLET, "Beware, this level is protected by %s!", name);
        }
    }
    else
    {
        /* Normal monsters */
        plural_aux(name);
        cmsg_format(TERM_VIOLET, "Be warned, this level is guarded by %d %s!", q_num, name);

    }
}


/*
 * Hack -- Check if a level is a "quest" level
 */
int quest_number(int level)
{
    int i;

    /* Check quests */
    if (p_ptr->inside_quest)
        return (p_ptr->inside_quest);

    for (i = 0; i < max_quests; i++)
    {
        if (quest[i].status != QUEST_STATUS_TAKEN) continue;

        if ((quest[i].type == QUEST_TYPE_KILL_LEVEL) &&
            !(quest[i].flags & QUEST_FLAG_PRESET) &&
            (quest[i].level == level) &&
            (quest[i].dungeon == dungeon_type))
            return (i);
    }
	int randnum = random_quest_number(level);
	if (randnum > 0) return randnum;
    /* Check for random quest */


	for (i = 0; i < max_quests; i++)
	{
		if (quest[i].status != QUEST_STATUS_TAKEN) continue;

		if (quest[i].max_num > 1 && QUEST_TYPE_KILL_LEVEL){
			if ((quest[i].dungeon == dungeon_type)){
				if ((quest[i].level <= level))
						return i;
			}
		}
	}
	/*Shitty warghack*/

	

	return 0;
}


/*
 * Return the index of the random quest on this level
 * (or zero)
 */
int random_quest_number(int level)
{
    int i;

    if (dungeon_type != DUNGEON_ANGBAND) return 0;

    for (i = MIN_RANDOM_QUEST; i < MIN_RANDOM_QUEST + num_random_quests; i++)
    {
        if ((quest[i].type == QUEST_TYPE_RANDOM) &&
            (quest[i].status == QUEST_STATUS_TAKEN) &&
            (quest[i].level == level) &&
            (quest[i].dungeon == DUNGEON_ANGBAND))
        {
            return i;
        }
    }
    /* Nope */
    return 0;
}
