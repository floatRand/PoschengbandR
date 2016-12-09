#include "angband.h"

#include <assert.h>

/* New code for object values. Designed by Dave.
   p is price
   q is ???
   y is ???
   w is base weapon cost
   a is base armor cost

   Anyway, the spec is here: http://code.google.com/p/chengband/wiki/ArtifactScoring
   I named variables to match.

   Update: Inspired by Dave, but I've mangled so much at this point that all the
   blame rightly belongs to me!
*/

debug_hook cost_calc_hook = NULL;

static double _calc_cost(double cost, int count)
{
    int c = count - 1;
    /* It gets harder to add new stuff to an enchanted item
    return cost * (1.0 + c/10.0 + c*c/50.0); */
    return cost * (1.0 + c/5.0 + c*c/25.0);
}

static double _check_flag_and_score(u32b flgs[OF_ARRAY_SIZE], u32b flg, u32b score, int *count)
{
    double result = 0.0;
    if (have_flag(flgs, flg))
    {
        (*count)++;
        result += _calc_cost(score, *count);
    }
    return result;
}

static s32b _activation_p(u32b flgs[OF_ARRAY_SIZE], object_type *o_ptr)
{
    /* Make sure player has learned the activation before scoring it! */
    if (have_flag(flgs, OF_ACTIVATE) && obj_has_effect(o_ptr))
    {
        effect_t effect = obj_get_effect(o_ptr);
        assert(effect.type);
        return effect_value(&effect);
    }
    return 0;
}


static s32b _aura_p(u32b flgs[OF_ARRAY_SIZE])
{
    s32b cost = 0;
    int count = 0;

    cost += _check_flag_and_score(flgs, OF_AURA_FIRE,     1000, &count);
    cost += _check_flag_and_score(flgs, OF_AURA_COLD,     1000, &count);
    cost += _check_flag_and_score(flgs, OF_AURA_ELEC,     1500, &count);
    cost += _check_flag_and_score(flgs, OF_AURA_SHARDS,   2000, &count);
    cost += _check_flag_and_score(flgs, OF_AURA_REVENGE,  5000, &count);

    return cost;
}

static s32b _stats_q(u32b flgs[OF_ARRAY_SIZE], int pval)
{
    s32b cost = 0;
    int count = 0;
    int mult;

    if (pval > 10)
        pval = 10;
    if (pval < -10)
        pval = -10;

    mult = pval + pval * ABS(pval) / 3; /* negative pvals should be removed ... */

    /* Stats */
    count = 0;
    cost += _check_flag_and_score(flgs, OF_STR,  600*mult, &count);
    cost += _check_flag_and_score(flgs, OF_INT,  500*mult, &count);
    cost += _check_flag_and_score(flgs, OF_WIS,  500*mult, &count);
    cost += _check_flag_and_score(flgs, OF_DEX,  550*mult, &count);
    cost += _check_flag_and_score(flgs, OF_CON,  600*mult, &count);
    cost += _check_flag_and_score(flgs, OF_CHR,  400*mult, &count);

    count = 0;
    cost -= _check_flag_and_score(flgs, OF_DEC_STR,  600*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_INT,  500*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_WIS,  500*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_DEX,  550*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_CON,  600*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_CHR,  400*mult, &count);

    /* Skills */
    count = 0;

    cost += _check_flag_and_score(flgs, OF_MAGIC_MASTERY,  1500*mult, &count);
    cost += _check_flag_and_score(flgs, OF_STEALTH,  1000*mult, &count);
    cost += _check_flag_and_score(flgs, OF_SPELL_CAP,  1000*mult, &count);
    cost += _check_flag_and_score(flgs, OF_SPELL_POWER,  2500*mult, &count);
    cost += _check_flag_and_score(flgs, OF_LIFE,  1000*mult, &count);


    count = 0;
    cost -= _check_flag_and_score(flgs, OF_DEC_MAGIC_MASTERY,  1500*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_STEALTH,  1000*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_SPELL_CAP,  1000*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_SPELL_POWER,  2500*mult, &count);
    cost -= _check_flag_and_score(flgs, OF_DEC_LIFE, 1000*mult, &count);

    cost -= _check_flag_and_score(flgs, OF_DEC_SPEED, 1000*pval*pval, &count);

    return cost;
}

static s32b _speed_p(int pval)
{
    int mult = pval * ABS(pval);

    return 1000 * mult;
}

static s32b _abilities_q(u32b flgs[OF_ARRAY_SIZE])
{
    double cost = 0.0;
    int count = 0;

    /* Weak Abilities */
    count = 0;
    cost += _check_flag_and_score(flgs, OF_THROWING, 100, &count);
    cost += _check_flag_and_score(flgs, OF_WARNING, 100, &count);
    cost += _check_flag_and_score(flgs, OF_LITE, 100, &count);
    cost += _check_flag_and_score(flgs, OF_DARKNESS, 100, &count);
    cost += _check_flag_and_score(flgs, OF_SLOW_DIGEST, 100, &count);
    cost += _check_flag_and_score(flgs, OF_SEE_INVIS, 500, &count);
    cost += _check_flag_and_score(flgs, OF_FREE_ACT, 750, &count);
    cost += _check_flag_and_score(flgs, OF_EASY_SPELL, 1000, &count);

    /* Low ESP */
    count = 0;
    cost += _check_flag_and_score(flgs, OF_ESP_ORC, 500, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_TROLL, 500, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_GIANT, 500, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_GOOD, 500, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_ANIMAL, 600, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_UNDEAD, 600, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_DEMON, 600, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_DRAGON, 700, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_HUMAN, 700, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_NONLIVING, 700, &count);

    /* Sustains */
    count = 0;
    cost += _check_flag_and_score(flgs, OF_SUST_STR, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_SUST_INT, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_SUST_WIS, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_SUST_DEX, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_SUST_CHR, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_SUST_CON, 1000, &count);

    /* Good Abilities */
    count = 0;
    cost += _check_flag_and_score(flgs, OF_LEVITATION, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_HOLD_LIFE, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_REGEN, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_LORE, 5000, &count);

    /* Great Abilities */
    count = 0;
	cost += _check_flag_and_score(flgs, OF_AUTOMAP, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_UNIQUE, 5000, &count);
    cost += _check_flag_and_score(flgs, OF_REFLECT, 5000, &count);
    cost += _check_flag_and_score(flgs, OF_ESP_EVIL, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_DEC_MANA, 10000, &count);
    cost += _check_flag_and_score(flgs, OF_TELEPATHY, 10000, &count);

    return (u32b) cost;

}

static s32b _brands_q(u32b flgs[OF_ARRAY_SIZE])
{
    double cost = 0.0;
    int count = 0;

    /* These are what I would expect to pay */
    cost += _check_flag_and_score(flgs, OF_BRAND_FIRE, 8000, &count);
    cost += _check_flag_and_score(flgs, OF_BRAND_COLD, 8000, &count);
    cost += _check_flag_and_score(flgs, OF_BRAND_ACID, 12000, &count);
    cost += _check_flag_and_score(flgs, OF_BRAND_ELEC, 15000, &count);

    return (u32b) cost;
}

static s32b _resistances_q(u32b flgs[OF_ARRAY_SIZE])
{
    double cost = 0.0;
    int count = 0;

    /* Low Resists */
    count = 0;
    cost += _check_flag_and_score(flgs, OF_RES_ACID, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_ELEC, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_FIRE, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_COLD, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_POIS, 5000, &count);

    /* High Resists */
    count = 0;
    cost += _check_flag_and_score(flgs, OF_RES_LITE, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_DARK, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_CONF, 3000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_NETHER, 4500, &count);
    cost += _check_flag_and_score(flgs, OF_RES_NEXUS, 4500, &count);
    cost += _check_flag_and_score(flgs, OF_RES_CHAOS, 6000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_SOUND, 6000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_SHARDS, 7000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_DISEN, 6000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_TIME, 10000, &count);

    /* Other Resists */
    count = 0;
    cost += _check_flag_and_score(flgs, OF_RES_BLIND, 1000, &count);
    cost += _check_flag_and_score(flgs, OF_RES_FEAR, 1000, &count);

    count = 0; /* Otherwise, immunities *and* lots of resists are absurd :) */
    cost += _check_flag_and_score(flgs, OF_IM_ACID,  25000, &count);
    cost += _check_flag_and_score(flgs, OF_IM_ELEC,  30000, &count);
    cost += _check_flag_and_score(flgs, OF_IM_FIRE,  30000, &count);
    cost += _check_flag_and_score(flgs, OF_IM_COLD,  35000, &count);

    count = 0;
    cost -= _check_flag_and_score(flgs, OF_VULN_ACID, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_ELEC, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_FIRE, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_COLD, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_POIS, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_LITE, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_DARK, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_BLIND, 2000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_CONF, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_NETHER, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_NEXUS, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_CHAOS, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_SOUND, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_SHARDS, 5000, &count);
    cost -= _check_flag_and_score(flgs, OF_VULN_DISEN, 5000, &count);

    return (u32b) cost;
}

s32b _finalize_p(s32b p, u32b flgs[OF_ARRAY_SIZE], object_type *o_ptr)
{
    char dbg_msg[512];
    s32b y;

    y = _activation_p(flgs, o_ptr);
    if (y != 0)
    {
        p += y;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Activation: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (obj_is_identified(o_ptr))
    {
        int xtra = 0;
        if (o_ptr->name2 == EGO_ROBE_TWILIGHT)
        {
            p = p / 3;
            if (cost_calc_hook)
            {
                sprintf(dbg_msg, "  * Hidden Power: p = %d", p);
                cost_calc_hook(dbg_msg);
            }
        }
        switch (o_ptr->name1)
        {
        case ART_STONE_OF_NATURE:
        case ART_STONE_OF_LIFE:
        case ART_STONE_OF_SORCERY:
        case ART_STONE_OF_CHAOS:
        case ART_STONE_OF_DEATH:
        case ART_STONE_OF_TRUMP:
        case ART_STONE_OF_DAEMON:
        case ART_STONE_OF_CRUSADE:
        case ART_STONE_OF_CRAFT:
        case ART_STONE_OF_ARMAGEDDON:
        case ART_STONE_OF_MIND:
		case ART_LCOIN:
            xtra = 5000;
            break;
        case ART_ASSASSINATOR:
		case ART_SOFT_TREAD:
            xtra = 25000;
            break;
        case ART_SPECTRAL_DSM: /* Passwall */
            xtra = 50000;
            break;
		case ART_AMULET_NIGHTMARE:
			p = p / 2;
			break;
        }
        if (xtra)
        {
            p += xtra;
            if (cost_calc_hook)
            {
                sprintf(dbg_msg, "  * Hidden Power: p = %d", p);
                cost_calc_hook(dbg_msg);
            }
        }
    }

    if (have_flag(flgs, OF_AGGRAVATE))
    {
        p = p * 8 / 10;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Aggravate: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (have_flag(flgs, OF_NO_TELE) && o_ptr->tval != TV_AMULET)
    {
        p = p * 7 / 10;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * No Tele: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (have_flag(flgs, OF_NO_MAGIC) && o_ptr->tval != TV_AMULET)
    {
        p = p * 9 / 10;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * No Magic: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (have_flag(flgs, OF_DRAIN_EXP))
    {
        p = p * 9 / 10;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Drain XP: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (have_flag(flgs, OF_TY_CURSE))
    {
        p = p * 5 / 10;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * AFC: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (obj_is_identified(o_ptr) && o_ptr->curse_flags & OFC_PERMA_CURSE)
    {
        p = p * 8 / 10;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Perm Curse: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (o_ptr->tval != TV_LITE && !object_is_jewelry(o_ptr))
    {
        /* Do we know this is an artifact? */
        if ( (obj_is_identified(o_ptr) && object_is_artifact(o_ptr))
          || (o_ptr->feeling & (FEEL_SPECIAL | FEEL_TERRIBLE)) )
        {
        }
        else
        {
            p = (p + 1) * 3 / 4;
            if (cost_calc_hook)
            {
                sprintf(dbg_msg, "  * Not Artifact: p = %d", p);
                cost_calc_hook(dbg_msg);
            }
        }
    }

    /* Negative values don't make much sense, and some code
       was using unsigned integers for values (e.g. Androids) */
    if (p <= 0)
    {
        p = 0;
        if (o_ptr->name1 || o_ptr->name2 || o_ptr->art_name)
            p = 1;
    }

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Result: %d", p);
        cost_calc_hook(dbg_msg);
    }

    return p;
}

s32b jewelry_cost(object_type *o_ptr, int options)
{
    s32b j, y, q, p;
    int  to_h = 0, to_d = 0, to_a = 0, pval = 0;
    u32b flgs[OF_ARRAY_SIZE];
    char dbg_msg[512];

    if (options & COST_REAL)
        obj_flags(o_ptr, flgs);
    else
        obj_flags_known(o_ptr, flgs);

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        to_h = o_ptr->to_h;
        to_d = o_ptr->to_d;
        to_a = o_ptr->to_a;
    }
    pval = o_ptr->pval;

    if (cost_calc_hook)
    {
        char buf[MAX_NLEN];
        object_desc(buf, o_ptr, 0);
        sprintf(dbg_msg, "Scoring `%s` ...", buf);
        cost_calc_hook(dbg_msg);
    }

    switch (o_ptr->tval)
    {
    case TV_LITE:
        j = 1000;
        break;
    case TV_RING:
        j = 400;
        break;
    case TV_AMULET:
        j = 800;
        break;
    default:
        return 0;
    }

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Base Cost: j = %d", j);
        cost_calc_hook(dbg_msg);
    }

    /* Resistances */
    q = _resistances_q(flgs);
    p = j + q;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Resistances: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Abilities */
    q = _abilities_q(flgs);
    if (have_flag(flgs, OF_NO_MAGIC)) q += 7000;
    if (have_flag(flgs, OF_NO_TELE)) q += 5000;
    if (have_flag(flgs, OF_NO_SUMMON)) q += 50000;
    p += q;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Abilities: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Brands */
    q = _brands_q(flgs);
    p += q;

    if (cost_calc_hook && q)
    {
        sprintf(dbg_msg, "  * Brands: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Other Bonuses */
    y = 0;
    if (have_flag(flgs, OF_SEARCH)) y += 100;
    if (have_flag(flgs, OF_INFRA)) y += 500;

    if (y != 0)
    {
        q = y*pval;
        p += q;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Other Crap: y = %d, q = %d, p = %d", y, q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Speed */
    if (have_flag(flgs, OF_SPEED))
    {
        p += _speed_p(pval);

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Speed: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (have_flag(flgs, OF_WEAPONMASTERY))
    {
        p += 10000 * pval;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Weaponmastery: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (have_flag(flgs, OF_BLOWS))
    {
        p += 15000 * pval;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Blows: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }
    if (have_flag(flgs, OF_DEC_BLOWS))
    {
        p -= 15000 * pval;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * -Blows: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (have_flag(flgs, OF_XTRA_SHOTS))
    {
        p += 7500 * pval;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Shots: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }
    if (have_flag(flgs, OF_XTRA_MIGHT))
    {
        p += 7500 * pval;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Extra Might: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Stats */
    q = _stats_q(flgs, pval);
    if (q != 0)
    {
        p += q;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Stats/Stealth: q = %d, p = %d", q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Auras */
    y = _aura_p(flgs);
    if (y != 0)
    {
        p += y;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Auras: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* +AC */
    if (to_a)
    {
        double x = to_a * ABS(to_a);
        double p2 = p;

        p2 = p2*(1000.0+x)/1000.0 + 20.0*x;
        p = (s32b)p2;

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * +AC: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* (+x,+y) */
    if (to_h != 0 || to_d != 0)
    {
        int x = to_h * ABS(to_h);
        int y = to_d * ABS(to_d);

        p += 100 * to_h + 10 * x;

        switch (o_ptr->name2)
        {
        case EGO_RING_WIZARDRY:
        case EGO_AMULET_MAGI:
        case EGO_RING_ARCHERY:
            p += 25 * y;
            break;
        default:
            /*p += 100 * y;*/
            p += 100 * to_d + 25 * to_d * ABS(to_d) + 3 * to_d * to_d * to_d;
        }

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * (+x,+y): p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    p = _finalize_p(p, flgs, o_ptr);
    return p;
}

s32b lite_cost(object_type *o_ptr, int options)
{
    s32b j, y, q, p;
    int  pval = 0;
    u32b flgs[OF_ARRAY_SIZE];
    char dbg_msg[512];

    if (options & COST_REAL)
        obj_flags(o_ptr, flgs);
    else
        obj_flags_known(o_ptr, flgs);

    pval = o_ptr->pval;

    switch (o_ptr->sval)
    {
    case SV_LITE_TORCH:
        j = 1;
        break;
    case SV_LITE_LANTERN:
        j = 30;
        break;
    case SV_LITE_FEANOR:
    default:
        j = 250;
        break;
    }

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Base Cost: j = %d", j);
        cost_calc_hook(dbg_msg);
    }

    /* These egos don't use flags for their effects ... sigh. */
    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        if (o_ptr->name2 == EGO_LITE_DURATION)
            j += 100;
    }

    /* Resistances */
    q = _resistances_q(flgs);
    p = j + q;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Resistances: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Abilities */
    q = _abilities_q(flgs);
    if (have_flag(flgs, OF_NO_MAGIC)) q += 7000;
    if (have_flag(flgs, OF_NO_TELE)) q += 5000;
    if (have_flag(flgs, OF_NO_SUMMON)) q += 1000000;
    p += q;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Abilities: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Speed */
    if (have_flag(flgs, OF_SPEED))
    {
        p += _speed_p(pval);

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Speed: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Stats */
    q = _stats_q(flgs, pval);
    if (q != 0)
    {
        p += q;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Stats/Stealth: q = %d, p = %d", q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Other Bonuses */
    y = 0;
    if (have_flag(flgs, OF_SEARCH)) y += 100;
    if (have_flag(flgs, OF_INFRA)) y += 500;

    if (y != 0)
    {
        q = y*pval;
        p += q;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Other Crap: y = %d, q = %d, p = %d", y, q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Stats */
    q = _stats_q(flgs, pval);
    if (q != 0)
    {
        p += q;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Stats/Stealth: q = %d, p = %d", q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Auras */
    y = _aura_p(flgs);
    if (y != 0)
    {
        p += y;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Auras: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    p = _finalize_p(p, flgs, o_ptr);
    return p;
}

s32b armor_cost(object_type *o_ptr, int options)
{
    s32b a, y, q, p;
    int  to_h = 0, to_d = 0, to_a = 0, pval = 0;
    u32b flgs[OF_ARRAY_SIZE];
    char dbg_msg[512];

    if (options & COST_REAL)
        obj_flags(o_ptr, flgs);
    else
        obj_flags_known(o_ptr, flgs);

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        to_h = o_ptr->to_h;
        to_d = o_ptr->to_d;
        to_a = o_ptr->to_a;
    }
    pval = o_ptr->pval;

    if (cost_calc_hook)
    {
        char buf[MAX_NLEN];
        object_desc(buf, o_ptr, 0);
        sprintf(dbg_msg, "Scoring `%s` ...", buf);
        cost_calc_hook(dbg_msg);
    }

    /* Base Cost */
    y = o_ptr->ac;
    if (y < 10)
        a = y * y * y;
    else
    {
        int x = y - 10;
        a = 1000 + 200*x + 20 * x * x;
    }
    if (object_is_(o_ptr, TV_CROWN, SV_GOLDEN_CROWN))
        a += 1000;
    else if (object_is_(o_ptr, TV_CROWN, SV_JEWELED_CROWN))
        a += 2000;
    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Base Cost: a = %d", a);
        cost_calc_hook(dbg_msg);
    }

    /* +AC ... Note, negative ac should decrease the cost! */
    if (have_flag(flgs, OF_IGNORE_ACID))
        a += 200*to_a + 20 * to_a * ABS(to_a);
    else
        a += 100*to_a + 10 * to_a * ABS(to_a);

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * +AC: a = %d", a);
        cost_calc_hook(dbg_msg);
    }

    /* Resistances */
    q = _resistances_q(flgs);
    p = a + q;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Resistances: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Abilities */
    q = _abilities_q(flgs);
    p += q;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Abilities: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Brands */
    q = _brands_q(flgs);
    p += q;

    if (cost_calc_hook && q)
    {
        sprintf(dbg_msg, "  * Brands: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Speed */
    if (have_flag(flgs, OF_SPEED))
    {
        p += _speed_p(pval);

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Speed: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Stats */
    q = _stats_q(flgs, pval);
    if (q != 0)
    {
        p += q;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Stats/Stealth: q = %d, p = %d", q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Auras */
    y = _aura_p(flgs);
    if (y != 0)
    {
        p += y;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Auras: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Extra Attacks */
    if (have_flag(flgs, OF_BLOWS))
    {
        p += 15000 * pval;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Blows: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }
    if (have_flag(flgs, OF_DEC_BLOWS))
    {
        p -= 15000 * pval;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * -Blows: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        if (have_flag(flgs, OF_DUAL_WIELDING))
        {
            p += 20000;
            if (cost_calc_hook)
            {
                sprintf(dbg_msg, "  * Genji: p = %d", p);
                cost_calc_hook(dbg_msg);
            }
        }
    }
    /* (+x,+y) */
    if (to_h != 0 || to_d != 0)
    {
        int x = to_h * ABS(to_h);
        int y = to_d * ABS(to_d);

        p += 100 * to_h + 10 * x;

        if (to_d > 20) /* Master Tonberry, Destroyer */
        {
            p += 40000; /* +20 damage */
            p += (to_d - 20) * 1000;
        }
        else
        {
            if (o_ptr->name2 == EGO_CROWN_MAGI)
                p += 25 * y;
            else
                p += 100 * y;
        }
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * (+x,+y): p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    p = _finalize_p(p, flgs, o_ptr);
    return p;
}

s32b weapon_cost(object_type *o_ptr, int options)
{
    s32b y, w, p, q;
    int  to_h = 0, to_d = 0, to_a = 0, pval = 0;
    u32b flgs[OF_ARRAY_SIZE];
    char dbg_msg[512];

    /* Hacks for objects with "hidden" powers */
    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_POISON_NEEDLE)
        return 2500;
    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_RUNESWORD)
        return 1;

    if (options & COST_REAL)
        obj_flags(o_ptr, flgs);
    else
        obj_flags_known(o_ptr, flgs);

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        to_h = o_ptr->to_h;
        to_d = o_ptr->to_d;
        to_a = o_ptr->to_a;
    }
    pval = o_ptr->pval;

    if (cost_calc_hook)
    {
        char buf[MAX_NLEN];
        object_desc(buf, o_ptr, 0);
        sprintf(dbg_msg, "Scoring `%s` ...", buf);
        cost_calc_hook(dbg_msg);
    }
    {
        object_kind *k_ptr = &k_info[o_ptr->k_idx];
        double b = (double)k_ptr->dd * ((double)k_ptr->ds + 1.0)/2;
        double d = (double)o_ptr->dd * ((double)o_ptr->ds + 1.0)/2;
        double x;
        double s = 1.0;

        /* Figure average damage per strike. Not really because we are stacking slays
           albeit weighted by my off the cuff estimates of utility */
        if (have_flag(flgs, OF_KILL_ORC)) s += (4.0 * .01);
        else if (have_flag(flgs, OF_SLAY_ORC)) s += (2.0 * .01);

        if (have_flag(flgs, OF_KILL_TROLL)) s += (4.0 * .02);
        else if (have_flag(flgs, OF_SLAY_TROLL)) s += (2.0 * .02);

        if (have_flag(flgs, OF_KILL_ANIMAL)) s += (3.0 * .1);
        else if (have_flag(flgs, OF_SLAY_ANIMAL)) s += (1.5 * .1);

        if (have_flag(flgs, OF_KILL_HUMAN)) s += (3.0 * .1);
        else if (have_flag(flgs, OF_SLAY_HUMAN)) s += (1.5 * .1);

        if (have_flag(flgs, OF_KILL_UNDEAD)) s += (4.0 * .1);
        else if (have_flag(flgs, OF_SLAY_UNDEAD)) s += (2.0 * .1);

        if (have_flag(flgs, OF_KILL_DEMON)) s += (4.0 * .15);
        else if (have_flag(flgs, OF_SLAY_DEMON)) s += (2.0 * .15);

        if (have_flag(flgs, OF_KILL_GIANT)) s += (4.0 * .075);
        else if (have_flag(flgs, OF_SLAY_GIANT)) s += (2.0 * 0.075);

        if (have_flag(flgs, OF_KILL_DRAGON)) s += (4.0 * .1);
        else if (have_flag(flgs, OF_SLAY_DRAGON)) s += (2.0 * .1);

        if (have_flag(flgs, OF_BRAND_POIS)) s += (1.5 * .1);
        if (have_flag(flgs, OF_BRAND_ACID)) s += (1.5 * .15);
        if (have_flag(flgs, OF_BRAND_ELEC)) s += (1.5 * .2);
        if (have_flag(flgs, OF_BRAND_FIRE)) s += (1.5 * .1);
        if (have_flag(flgs, OF_BRAND_COLD)) s += (1.5 * .1);

        if (have_flag(flgs, OF_BRAND_CHAOS)) s += 0.2;
        if (have_flag(flgs, OF_BRAND_VAMP)) s += 0.1; /* Not really a slay, but vamp works better on higher dice */

        if (have_flag(flgs, OF_KILL_EVIL)) s += (2.5 * 0.8);
        else if (have_flag(flgs, OF_SLAY_EVIL)) s += (1.0 * 0.8);

        if (o_ptr->name1 == ART_ETERNAL_BLADE) s += (1.0 * 0.8); /* BR_TIME */

        if (have_flag(flgs, OF_SLAY_GOOD)) s += (1.0 * 0.20);
        if (have_flag(flgs, OF_SLAY_LIVING)) s += (1.0 * 0.70);

        if (have_flag(flgs, OF_BRAND_MANA))
        {
            s = (s * 1.50 + 1.0) * 0.25 + s * 0.75;
        }

        if (have_flag(flgs, OF_VORPAL2))
            s *= 1.67;
        else if (have_flag(flgs, OF_VORPAL))
            s *= 1.22;

        if (have_flag(flgs, OF_STUN))
            s *= 1.10;

        d = d*s + (double)to_d;
        if (d < 1.0)
            d = 1.0;

		if (have_flag(flgs, OF_BLOWS))
			d += (d + 40.0)*pval / 10.0;
		else if (have_flag(flgs, OF_DEC_BLOWS))
			d -= (d + 40.0)*pval / 10.0;
                   /* ^---Give extra attacks more respect ... Each pval is +0.50 blows. */
		if (d < 1.0)
			d = 1.0;

        x = d - b;

        /*         v----Extra damage over the base object type has a strong linear component, so
                   v    that weaker to mid range weapons get better separation. */
        w = (s32b)(x * 100.0) + (s32b)(x * x * 5.0) + (s32b)(d * d * d * 0.2);
        /* While raw damage output continues to be cubic-----^           ^
           But, scaled to gain cross type power comparability------------^ */

        if (have_flag(flgs, OF_BRAND_VAMP))
            w += 3000;

        if (have_flag(flgs, OF_IMPACT))
            w += 250;

        if (have_flag(flgs, OF_BRAND_WILD))
            w += 10000;

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Damage: d = %.2f, b = %.2f, s = %.2f, x = %.2f, w = %d", d, b, s, x, w);
            cost_calc_hook(dbg_msg);
        }
    }

    if (to_h <= 10)
        w += 100 * to_h;
    else
        w += 10 * to_h * to_h;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Accuracy: w = %d", w);
        cost_calc_hook(dbg_msg);
    }

    /* Resistances */
    q = _resistances_q(flgs) * 7 / 10;
    p = w + q;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Resistances: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Abilities */
    q = _abilities_q(flgs);
    p += q;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Abilities: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Speed */
    if (have_flag(flgs, OF_SPEED))
    {
        p += _speed_p(pval);
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Speed: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Stats */
    q = _stats_q(flgs, pval);
    if (q != 0)
    {
        p += q;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Stats/Stealth: q = %d, p = %d", q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Other Bonuses */
    y = 0;
    if (have_flag(flgs, OF_SEARCH)) y += 100;
    if (have_flag(flgs, OF_INFRA)) y += 500;
    if (have_flag(flgs, OF_TUNNEL))
    {
        if (o_ptr->tval == TV_DIGGING && pval == 1)
        {
            /* ?? Shovels and picks ... */
            y += 150;
        }
        else
            y += 1000;
    }

    if (y != 0)
    {
        q = y*pval;
        p += q;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Other Crap: y = %d, q = %d, p = %d", y, q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Auras */
    y = _aura_p(flgs);
    if (y != 0)
    {
        p += y;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Auras: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* AC Bonus */
    if (to_a != 0)
    {
    /*    p += 500*o_ptr->to_a + o_ptr->to_a * ABS(o_ptr->to_a) * 30; */
        p += 500*to_a;

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * AC: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    if (object_allow_two_hands_wielding(o_ptr))
        p += 75;

    if (o_ptr->weight > 20)
        p += o_ptr->weight - 20;

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Weight/2-hands: p = %d", p);
        cost_calc_hook(dbg_msg);
    }

    p = _finalize_p(p, flgs, o_ptr);
    return p;
}

s32b ammo_cost(object_type *o_ptr, int options)
{
    /* TODO: Implement this properly. Many weapon flags will not apply here.
       Also, some egos, like EGO_AMMO_RETURNING will not score properly */
    s32b result = weapon_cost(o_ptr, options);
    result /= 25;
    if (!result)
        result = 1;
    return result;
}

static s32b _avg_dam_bow(object_type *o_ptr, int options)
{
    s32b d = 0;
    s32b m = o_ptr->mult;
    int  to_d = 0;

    if ((options & COST_REAL) || object_is_known(o_ptr))
        to_d = o_ptr->to_d;

    switch (o_ptr->sval)
    {
    case SV_SLING:
        d = m*(2 + to_d) / 100;
        break;

    case SV_SHORT_BOW:
        d = m*(2 + to_d) / 100;
        break;

    case SV_LONG_BOW:
        d = m*(3 + to_d) / 100;
        break;

    case SV_NAMAKE_BOW:
        d = m*(3 + to_d) / 100;
        break;

    case SV_LIGHT_XBOW:
        d = m*(4 + to_d) / 100;
        break;

    case SV_HEAVY_XBOW:
        d = m*(4 + to_d) / 100;
        break;

    case SV_HARP:
        d = 10;
        break;

    default:
        d = 10; /* Gun */
    }
    d = d * 10000 / bow_energy(o_ptr->sval);

    return MAX(0, d);
}

s32b bow_cost(object_type *o_ptr, int options)
{
    s32b y, w, p, q, t;
    int  to_h = 0, to_a = 0, pval = 0;
    u32b flgs[OF_ARRAY_SIZE];
    char dbg_msg[512];

    if (options & COST_REAL)
        obj_flags(o_ptr, flgs);
    else
        obj_flags_known(o_ptr, flgs);

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        to_h = o_ptr->to_h;
        to_a = o_ptr->to_a;
    }
    pval = o_ptr->pval;

    if (cost_calc_hook)
    {
        char buf[MAX_NLEN];
        object_desc(buf, o_ptr, 0);
        sprintf(dbg_msg, "Scoring `%s` ...", buf);
        cost_calc_hook(dbg_msg);
    }

    /* Base Cost calculated from expected damage output */
    t = _avg_dam_bow(o_ptr, options);
    if (have_flag(flgs, OF_BRAND_POIS)) t = t * 5 / 4;
    if (have_flag(flgs, OF_BRAND_ACID)) t = t * 5 / 4;
    if (have_flag(flgs, OF_BRAND_ELEC)) t = t * 5 / 4;
    if (have_flag(flgs, OF_BRAND_FIRE)) t = t * 5 / 4;
    if (have_flag(flgs, OF_BRAND_COLD)) t = t * 5 / 4;

    w = (t * t / 11) * t;
    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Base Cost: w = %d", w);
        cost_calc_hook(dbg_msg);
    }

    if (have_flag(flgs, OF_XTRA_SHOTS))
    {
        w += w * pval / 4; /* +.25 shots per pval */
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Extra Shots: w = %d", w);
            cost_calc_hook(dbg_msg);
        }
    }

    /* (+x,+y) */
    if (to_h < 0) {}
    else if (to_h <= 10)
        w += 100 * to_h;
    else
    {
        w += 100 * to_h;
        /*w += 10 * o_ptr->to_h * o_ptr->to_h;*/
    }

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Accuracy: w = %d", w);
        cost_calc_hook(dbg_msg);
    }

    /* Resistances */
    q = _resistances_q(flgs);
    p = w + q + (q/100)*w/200;
    /*p = w + q*(1+w/20000);*/

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Resistances: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Abilities */
    q = _abilities_q(flgs);
    p += q + (q/100)*w/400;
    /*p += q*(1+w/20000);*/

    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Abilities: q = %d, p = %d", q, p);
        cost_calc_hook(dbg_msg);
    }

    /* Speed */
    if (have_flag(flgs, OF_SPEED))
    {
        p += _speed_p(pval);

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Speed: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Stats */
    q = _stats_q(flgs, pval);
    if (q != 0)
    {
        p += q;
        /*p += q*(1 + w/10000);*/
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Stats/Stealth: q = %d, p = %d", q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Other Bonuses */
    y = 0;
    if (have_flag(flgs, OF_SEARCH)) y += 100;
    if (have_flag(flgs, OF_INFRA)) y += 500;
    if (y != 0)
    {
        q = y*pval;
        p += q + (q/100)*w/300;
        /*p += q*(1 + w/30000);*/
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Other Crap: y = %d, q = %d, p = %d", y, q, p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* Auras */
    y = _aura_p(flgs);
    if (y != 0)
    {
        p += y;
        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * Auras: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    /* AC Bonus */
    if (to_a != 0)
    {
        p += 500*to_a + to_a * ABS(to_a) * 30;

        if (cost_calc_hook)
        {
            sprintf(dbg_msg, "  * AC: p = %d", p);
            cost_calc_hook(dbg_msg);
        }
    }

    p += o_ptr->weight; /* Hack for average gear ... */
    if (cost_calc_hook)
    {
        sprintf(dbg_msg, "  * Weight: p = %d", p);
        cost_calc_hook(dbg_msg);
    }

    p = _finalize_p(p, flgs, o_ptr);
    return p;
}

s32b new_object_cost(object_type *o_ptr, int options)
{
    if (object_is_melee_weapon(o_ptr)) return weapon_cost(o_ptr, options);
    else if (o_ptr->tval == TV_BOW) return bow_cost(o_ptr, options);
    else if (object_is_ammo(o_ptr)) return ammo_cost(o_ptr, options);
    else if (object_is_armour(o_ptr) || object_is_shield(o_ptr)) return armor_cost(o_ptr, options);
    else if (object_is_jewelry(o_ptr) || (o_ptr->tval == TV_LITE && object_is_artifact(o_ptr))) return jewelry_cost(o_ptr, options);
    else if (o_ptr->tval == TV_LITE) return lite_cost(o_ptr, options);
    else if (object_is_device(o_ptr)) return device_value(o_ptr, options);
    return 0;
}
