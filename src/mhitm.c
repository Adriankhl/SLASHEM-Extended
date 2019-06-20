/*	SCCS Id: @(#)mhitm.c	3.4	2003/01/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include "edog.h"

extern boolean notonhead;
extern const char *breathwep[];		/* from mthrowu.c */

#define POLE_LIM 8	/* How far monsters can use pole-weapons */

#ifdef OVLB

static NEARDATA boolean vis, far_noise;
static NEARDATA long noisetime;
static NEARDATA struct obj *otmp;

static const char brief_feeling[] =
	"have a %s feeling for a moment, then it passes.";

STATIC_DCL char *mon_nam_too(char *,struct monst *,struct monst *);
STATIC_DCL void mrustm(struct monst *, struct monst *, struct obj *);
STATIC_DCL int breamm(struct monst *, struct monst *, struct attack *);
STATIC_DCL int spitmm(struct monst *, struct monst *, struct attack *);
STATIC_DCL int thrwmm(struct monst *, struct monst *);
STATIC_DCL int hitmm(struct monst *,struct monst *,struct attack *);
STATIC_DCL int gazemm(struct monst *,struct monst *,struct attack *);
STATIC_DCL int gulpmm(struct monst *,struct monst *,struct attack *);
STATIC_DCL int explmm(struct monst *,struct monst *,struct attack *);
STATIC_DCL int mdamagem(struct monst *,struct monst *,struct attack *);
STATIC_DCL void mswingsm(struct monst *, struct monst *, struct obj *);
STATIC_DCL void noises(struct monst *,struct attack *);
STATIC_DCL void missmm(struct monst *,struct monst *, int, int, struct attack *);
STATIC_DCL int passivemm(struct monst *, struct monst *, BOOLEAN_P, int, int);
STATIC_DCL void stoogejoke();

STATIC_PTR void set_lit(int,int,void *);

/* Needed for the special case of monsters wielding vorpal blades (rare).
 * If we use this a lot it should probably be a parameter to mdamagem()
 * instead of a global variable.
 */
static int dieroll;

static const char *random_joke[] = {
	"Why I ought a ...",
	"You'll get what's comming!",
	"I'll murder you!",
	"I get no respect!",
	"Right in the kisser!",
	"Wait a minute!",
	"Take it easy!",
	"Alright already!",
	"That's more like it!",
	"Well excuse me!",
	"Take that!",
	"I'll fix you!",
	"I'm sorry!",
	"Your mama!",
	"Shut up!",
	"Listen you!",
	"Pardon me!",
	"Not that!",
	"Quiet!",
	"Relax!",
	"Certainly!",
	"Ouch!",
	"What happened?",
	"What was that for?",
	"What's the matter with you?",
	"Oh Yea?",
	"Wise guy eh?",
	"How about a knuckle sandwich?",
	"You coward!",
	"You rat you!",
	"You chuckelhead!",
	"You bonehead!",
	"You numbskull!",
	"Nyak Nyak Nyak ...",
	"Woop Woop Woop Woop ..."
};

/* returns mon_nam(mon) relative to other_mon; normal name unless they're
   the same, in which case the reference is to {him|her|it} self */
STATIC_OVL char *
mon_nam_too(outbuf, mon, other_mon)
char *outbuf;
struct monst *mon, *other_mon;
{
	strcpy(outbuf, mon_nam(mon));
	if (mon == other_mon)
	    switch (pronoun_gender(mon)) {
	    case 0:	strcpy(outbuf, "himself");  break;
	    case 1:	strcpy(outbuf, "herself");  break;
	    default:	strcpy(outbuf, "itself"); break;
	    }
	return outbuf;
}

STATIC_OVL void
noises(magr, mattk)
	register struct monst *magr;
	register struct	attack *mattk;
{
	boolean farq = (distu(magr->mx, magr->my) > 15);

	if(flags.soundok && (farq != far_noise || moves-noisetime > 10)) {
		far_noise = farq;
		noisetime = moves;
		You_hear("%s%s.",
			(mattk->aatyp == AT_EXPL) ? "an explosion" : "some noises",
			farq ? " in the distance" : "");
	}
}

STATIC_OVL
void
missmm(magr, mdef, target, roll, mattk)
	register struct monst *magr, *mdef;
	struct attack *mattk;
	int target, roll;
{
	boolean nearmiss = (target == roll);
	const char *fmt;
        char buf[BUFSZ], mon_name[BUFSZ];

	register struct obj *blocker = (struct obj *)0;
	long mwflags = mdef->misc_worn_check;

		/* 3 values for blocker
		 *	No blocker:  (struct obj *) 0  
		 * 	Piece of armour:  object
		 */

	/* This is a hack,  since there is no fast equivalent for uarm, uarms, etc.  
	 * Technically, we really should check from the inside out...
	 */
	if (target < roll) {
	    for (blocker = mdef->minvent; blocker; blocker = blocker->nobj) {
		if (blocker->owornmask & mwflags) {
			target += ARM_BONUS(blocker);
			if (target > roll) break;
		}
	    }
	}

	if (vis) {
		if (!canspotmon(magr))
		    map_invisible(magr->mx, magr->my);
		if (!canspotmon(mdef))
		    map_invisible(mdef->mx, mdef->my);
		if (mdef->m_ap_type) seemimic(mdef);
		if (magr->m_ap_type) seemimic(magr);
		if (flags.verbose && !nearmiss && blocker) {
			fmt = "%s %s blocks";
			sprintf(buf,fmt, s_suffix(Monnam(mdef)), 
				aobjnam(blocker, (char *)0));
	                pline("%s %s.", buf, mon_nam_too(mon_name, magr, mdef));
		} else {
		fmt = (could_seduce(magr,mdef,mattk) && !magr->mcan) ?
				"%s pretends to be friendly to" : 
				((flags.verbose && nearmiss) ? "%s just misses" : 
				  "%s misses");
		sprintf(buf, fmt, Monnam(magr));
	                pline("%s %s.", buf, mon_nam_too(mon_name, mdef, magr));
		}
	} else  noises(magr, mattk);
}

/*
 *  fightm()  -- fight some other monster
 *
 *  Returns:
 *	0 - Monster did nothing.
 *	1 - If the monster made an attack.  The monster might have died.
 *
 *  There is an exception to the above.  If mtmp has the hero swallowed,
 *  then we report that the monster did nothing so it will continue to
 *  digest the hero.
 */
int
fightm(mtmp)		/* have monsters fight each other */
	register struct monst *mtmp;
{
	register struct monst *mon, *nmon;
	int result, has_u_swallowed;
#ifdef LINT
	nmon = 0;
#endif
	/* perhaps the monster will resist Conflict */
	if(resist(mtmp, RING_CLASS, 0, 0))
	    return(0);
	if(resist(mtmp, RING_CLASS, 0, 0))
	    return(0);
	if (!rn2(2)) return(0);
	/* they're now highly resistant to conflict, because otherwise things would be too easy --Amy */

	if(u.ustuck == mtmp) {
	    /* perhaps we're holding it... */
	    if(itsstuck(mtmp))
		return(0);
	}
	has_u_swallowed = (u.uswallow && (mtmp == u.ustuck));

	for(mon = fmon; mon; mon = nmon) {
	    nmon = mon->nmon;
	    if(nmon == mtmp) nmon = mtmp->nmon;
	    /* Be careful to ignore monsters that are already dead, since we
	     * might be calling this before we've cleaned them up.  This can
	     * happen if the monster attacked a cockatrice bare-handedly, for
	     * instance.
	     */
	    if(mon != mtmp && !DEADMONSTER(mon)) {
		if(monnear(mtmp,mon->mx,mon->my)) {
		    if(!u.uswallow && (mtmp == u.ustuck)) {
			if(!rn2(4)) {
			    pline("%s releases you!", Monnam(mtmp));
			    setustuck(0);
			} else
			    break;
		    }

		    /* mtmp can be killed */
		    bhitpos.x = mon->mx;
		    bhitpos.y = mon->my;
		    notonhead = 0;
		    result = mattackm(mtmp,mon);

		    if (result & MM_AGR_DIED) return 1;	/* mtmp died */
		    /*
		     *  If mtmp has the hero swallowed, lie and say there
		     *  was no attack (this allows mtmp to digest the hero).
		     */
		    if (has_u_swallowed) return 0;

		    /* Allow attacked monsters a chance to hit back. Primarily
		     * to allow monsters that resist conflict to respond.
		     */
		    if ((result & MM_HIT) && !(result & MM_DEF_DIED) &&
			rn2(4) && mon->movement >= NORMAL_SPEED) {
			mon->movement -= NORMAL_SPEED;
			notonhead = 0;
			(void) mattackm(mon, mtmp);	/* return attack */
		    }

		    return ((result & MM_HIT) ? 1 : 0);
		}
	    }
	}
	return 0;
}

/*
 * mattackm() -- a monster attacks another monster.
 *
 * This function returns a result bitfield:
 *
 *	    --------- aggressor died
 *	   /  ------- defender died
 *	  /  /  ----- defender was hit
 *	 /  /  /
 *	x  x  x
 *
 *	0x4	MM_AGR_DIED
 *	0x2	MM_DEF_DIED
 *	0x1	MM_HIT
 *	0x0	MM_MISS
 *
 * Each successive attack has a lower probability of hitting.  Some rely on the
 * success of previous attacks.  ** this doen't seem to be implemented -dl **
 *
 * In the case of exploding monsters, the monster dies as well.
 */
int
mattackm(magr, mdef)
    register struct monst *magr,*mdef;
{
    int		    i,		/* loop counter */
		    tmp,	/* amour class difference */
		    strike,	/* hit this attack */
		    attk,	/* attack attempted this time */
		    struck = 0,	/* hit at least once */
		    res[NATTK];	/* results of all attacks */
    int magrlev, magrhih; /* for to-hit calculations */
    struct attack   *mattk, alt_attk;
    struct permonst *pa, *pd;
    struct attack *a;
    struct permonst *mdat2;
    /*
     * Pets don't use "ranged" attacks for fear of hitting their master
     */
    boolean range;

    if (!magr || !mdef) return(MM_MISS);		/* mike@genat */
    if (!magr->mcanmove || magr->msleeping) return(MM_MISS);
    pa = magr->data;  pd = mdef->data;

    /* Grid bugs cannot attack at an angle. */
    if ((isgridbug(pa) || (uwep && uwep->oartifact == ART_EGRID_BUG && magr->data->mlet == S_XAN) || (uarmf && !rn2(10) && itemhasappearance(uarmf, APP_CHESS_BOOTS) ) ) && magr->mx != mdef->mx && magr->my != mdef->my)
	return(MM_MISS);

    range = (!magr->mtame || rn2(3)) && !monnear(magr, mdef->mx, mdef->my);

    /* Calculate the armour class differential. */
    tmp = find_mac(mdef);

    /* To-hit based on the monster's level. Nerf by Amy: high-level monsters shouldn't autohit. */
    magrlev = magr->m_lev;
    if (magrlev > 19) {
	magrhih = magrlev - 19;
	magrlev -= rnd(magrhih);
    if (magrlev > 29) {
	magrhih = magrlev - 19;
		if (magrhih > 1) magrhih /= 2;
		magrlev -= magrhih;
	}
    }
	/* pets need to be subjected to penalties or they'll be overpowered :P --Amy */
    if (magr->mtame) {
	if (magr->mflee) tmp -= 20;
	if (magr->mstun) tmp -= rnd(20);
	if (magr->mconf) tmp -= rnd(5);
	if (magr->mblinded && haseyes(magr->data)) tmp -= rnd(8);
	if (mdef->minvis && haseyes(magr->data) && !perceives(magr->data)) tmp -= 10;
	if (mdef->minvisreal) tmp -= (haseyes(magr->data) ? 30 : 20);
    } /* attacking monster is tame */

	/* monster attacks should be fully effective against pets so you can't just cheese out everything --Amy
	 * (basically, symmetrical with uhitm.c but only if the attack targets your pet; this isn't FIQslex) */
    if (mdef->mtame) {
	if (!rn2(5) && (level_difficulty() > 20) && magrlev < 5) magrlev = 5;
	if (!rn2(5) && (level_difficulty() > 40) && magrlev < 10) magrlev = 10;
	if (!rn2(5) && (level_difficulty() > 60) && magrlev < 15) magrlev = 15;
	if (!rn2(5) && (level_difficulty() > 80) && magrlev < 20) magrlev = 20;

	if (level_difficulty() > 5 && magrlev < 5 && !rn2(5)) magrlev++;
	if (level_difficulty() > 10 && magrlev < 5 && !rn2(2)) magrlev++;
	if (level_difficulty() > 10 && magrlev < 10 && !rn2(5)) magrlev++;
	if (level_difficulty() > 20 && magrlev < 10 && !rn2(2)) magrlev++;
	if (level_difficulty() > 20 && magrlev < 15 && !rn2(5)) magrlev++;
	if (level_difficulty() > 40 && magrlev < 15 && !rn2(2)) magrlev++;
	if (level_difficulty() > 30 && magrlev < 20 && !rn2(5)) magrlev++;
	if (level_difficulty() > 60 && magrlev < 20 && !rn2(2)) magrlev++;

	if (magr->egotype_hitter) tmp += 10;
	if (magr->egotype_piercer) tmp += 25;
	if (!(mdef->mcanmove)) tmp += 4;
	if (mdef->mtrapped) tmp += 2;

	if (magr->data == &mons[PM_IVORY_COAST_STAR]) tmp += 30; /* this monster is aiming abnormally well */
	if (magr->data == &mons[PM_HAND_OF_GOD]) tmp += 100; /* God personally is guiding this one's blows */
	if (magr->data == &mons[PM_FIRST_DUNVEGAN]) tmp += 100; /* this monster also almost always hits */
	if (magr->data == &mons[PM_DNETHACK_ELDER_PRIEST_TM_]) tmp += rnd(100); /* the elder priest uses an aimbot and a wallhack */

	if (magr->data->msound == MS_FART_LOUD && !magr->butthurt) tmp += 5;
	if (magr->data->msound == MS_FART_NORMAL && !magr->butthurt) tmp += 10;
	if (magr->data->msound == MS_FART_QUIET && !magr->butthurt) tmp += 15;
	if (magr->data->msound == MS_WHORE && !magr->butthurt) tmp += rnd(20);
	if (magr->data->msound == MS_SHOE) tmp += rnd(20);
	if (magr->data->msound == MS_STENCH) tmp += rnd(20);
	if (magr->data->msound == MS_CONVERT) tmp += rnd(10);
	if (magr->data->msound == MS_HCALIEN) tmp += rnd(25);
	if (magr->egotype_farter) tmp += 15;
	if (magr->fartbonus) tmp += magr->fartbonus;
	if (magr->crapbonus) tmp += magr->crapbonus;
	if (is_table(magr->mx, magr->my)) tmp += 3;
	if (humanoid(magr->data) && is_female(magr->data) && attacktype(magr->data, AT_KICK) && FemaleTrapMadeleine) tmp += 100;
	if (humanoid(magr->data) && is_female(magr->data) && FemaleTrapWendy) tmp += rnd(20);

	if (!rn2(20)) tmp += 20; /* "natural 20" like in D&D --Amy */

	if(!magr->cham && (is_demon(magr->data) || magr->egotype_gator) && monnear(magr, mdef->mx, mdef->my) && magr->data != &mons[PM_BALROG]
	   && magr->data != &mons[PM_SUCCUBUS]
	   && magr->data != &mons[PM_INCUBUS]
 	   && magr->data != &mons[PM_NEWS_DAEMON]
 	   && magr->data != &mons[PM_PRINTER_DAEMON]) {
		if(!magr->mcan && !rn2(magr->data == &mons[PM_PERCENTI_OPENS_A_GATE_] ? 5 : magr->data == &mons[PM_PERCENTI_PASSES_TO_YOU_] ? 5 : 23)) {
			msummon(magr, TRUE);
			pline("%s opens a gate!", Monnam(magr) );
			if (PlayerHearsSoundEffects) pline(issoviet ? "Sovetskaya nadeyetsya, chto demony zapolnyayut ves' uroven' i ubit' vas." : "Pitschaeff!");
		}
	 }

	if(!magr->cham && is_were(magr->data) && monnear(magr, mdef->mx, mdef->my)) {

	    if(!rn2(10) && !magr->mcan) {
	    	int numseen, numhelp;
		char buf[BUFSZ], genericwere[BUFSZ];

		strcpy(genericwere, "creature");
		numhelp = were_summon(magr->data, FALSE, &numseen, genericwere, TRUE);
		pline("%s summons help!", Monnam(magr));
		if (numhelp > 0) {
		    if (numseen == 0)
			You_feel("hemmed in.");
		} else pline("But none comes.");
	    }
	}

    } /* defending monster is tame */

    tmp += magrlev;

    if (mdef->mconf || !mdef->mcanmove || mdef->msleeping) {
	tmp += 4;
	mdef->msleeping = 0;
    }

	if (mdef->data == &mons[PM_DNETHACK_ELDER_PRIEST_TM_]) {
		mdef->isegotype = 1;
		mdef->egotype_covetous = 1;
	}

    /* undetect monsters become un-hidden if they are attacked */
    if (mdef->mundetected) {
	mdef->mundetected = 0;
	newsym(mdef->mx, mdef->my);
	if(canseemon(mdef) && !sensemon(mdef)) {
	    if (u.usleep) You("dream of %s.",
				(mdef->data->geno & G_UNIQ) ?
				a_monnam(mdef) : makeplural(m_monnam(mdef)));
	    else pline("Suddenly, you notice %s.", a_monnam(mdef));
	}
    }

    /* Elves hate orcs. */
    if (is_elf(pa) && is_orc(pd)) tmp++;


    /* Set up the visibility of action */
    vis = (cansee(magr->mx,magr->my) && cansee(mdef->mx,mdef->my) && (canspotmon(magr) || canspotmon(mdef)));

    /*	Set flag indicating monster has moved this turn.  Necessary since a
     *	monster might get an attack out of sequence (i.e. before its move) in
     *	some cases, in which case this still counts as its move for the round
     *	and it shouldn't move again.
     */
    magr->mlstmv = monstermoves;

    /* Now perform all attacks for the monster. */
    for (i = 0; i < NATTK; i++) {
	res[i] = MM_MISS;
	mattk = getmattk(pa, i, res, &alt_attk);
	otmp = (struct obj *)0;
	attk = 1;

	switch (mattk->aatyp) {
	    case AT_BREA:
	    case AT_SPIT:

		if (range && mdef->mtame && !linedup(magr->mx,magr->my,mdef->mx,mdef->my) ) {
		    strike = 0;
		    attk = 0;
		    break;

		}

		if (range || (mattk->aatyp == AT_SPIT && mdef->mtame) || (mdef->mtame && !mon_reflects(mdef, (char *)0) ) ) {

			if (!rn2(3)) {
				goto meleeattack;
			}
		    if (mattk->aatyp == AT_BREA) {

			if (mattk->adtyp == AD_FIRE && resists_fire(mdef)) {
				strike = 0;
			} else if (mattk->adtyp == AD_COLD && resists_cold(mdef)) {
				strike = 0;
			} else if (mattk->adtyp == AD_SLEE && resists_sleep(mdef)) {
				strike = 0;
			} else if (mattk->adtyp == AD_DISN && resists_disint(mdef)) {
				strike = 0;
			} else if (mattk->adtyp == AD_ELEC && resists_elec(mdef)) {
				strike = 0;
			} else if (mattk->adtyp == AD_DRST && resists_poison(mdef)) {
				strike = 0;
			} else if (mattk->adtyp == AD_ACID && resists_acid(mdef)) {
				strike = 0;
			} else if ((mattk->adtyp < AD_MAGM || mattk->adtyp > AD_SPC2) && mattk->adtyp != AD_RBRE) {
				goto meleeattack;
			}
			else res[i] = breamm(magr, mdef, mattk);
		    } else {
			if (mattk->adtyp == AD_ACID || mattk->adtyp == AD_BLND || mattk->adtyp == AD_TCKL || mattk->adtyp == AD_DRLI || mattk->adtyp == AD_NAST) {
				res[i] = spitmm(magr, mdef, mattk);
			} else goto meleeattack;

		    }
		    /* We can't distinguish no action from failed attack
		     * so assume defender doesn't waken unless actually hit.
		     */
		    strike = res[i] & MM_HIT;
		} else
		    strike = 0;
		attk = 0;
		break;

	    case AT_MAGC:

		if (range && mdef->mtame && !linedup(magr->mx,magr->my,mdef->mx,mdef->my) ) {
		    strike = 0;
		    attk = 0;
		    break;

		}

		/* [ALI] Monster-on-monster spell casting always fails. This
		 * is partly for balance reasons and partly because the
		 * amount of code required to implement it is prohibitive.
		 */
		/*strike = 0;
		attk = 0;*/
		if (canseemon(magr) && couldsee(magr->mx, magr->my)) {
		    char buf[BUFSZ];
		    strcpy(buf, Monnam(magr));
		    if (vis)
			pline("%s points at %s, then curses.", buf,
				mon_nam(mdef));
		    else
			pline("%s points and curses at something.", buf);
		} else if (flags.soundok)
		    Norep("You hear a mumbled curse.");

		goto meleeattack;
		break;

	    case AT_WEAP:
		/* "ranged" attacks */
#ifdef REINCARNATION
		if (!Is_rogue_level(&u.uz) && range) {
#else
		if (range || (!rn2(4) && mdef->mtame) ) {
#endif

		    res[i] = thrwmm(magr, mdef);
		    attk = 0;
		    strike = res[i] & MM_HIT;
		    break;
		}
		/* "hand to hand" attacks */
		if (magr->weapon_check == NEED_WEAPON || !MON_WEP(magr)) {
		    magr->weapon_check = NEED_HTH_WEAPON;
		    if (mon_wield_item(magr) != 0) {
			return 0;
		    }
		}
		possibly_unwield(magr, FALSE);
		otmp = MON_WEP(magr);

		if (otmp) {
		    if (vis) mswingsm(magr, mdef, otmp);
		    tmp += hitval(otmp, mdef);
		}
		/* fall through */
	    case AT_CLAW:
	    case AT_KICK:
	    case AT_BITE:
	    case AT_STNG:
	    case AT_TUCH:
	    case AT_BUTT:
	    case AT_LASH:
	    case AT_TRAM:
	    case AT_SCRA:
	    case AT_TENT:
	    case AT_BEAM:
meleeattack:
		/* Nymph that teleported away on first attack? */
		if ((distmin(magr->mx,magr->my,mdef->mx,mdef->my) > 1) && mattk->aatyp != AT_BREA && mattk->aatyp != AT_SPIT && mattk->aatyp != AT_MAGC && (mattk->aatyp != AT_BEAM || (mattk->aatyp == AT_BEAM && !linedup(magr->mx,magr->my,mdef->mx,mdef->my)) ) ) {
		    strike = 0;
		    break;
		}
		/* Monsters won't attack cockatrices physically if they
		 * have a weapon instead.  This instinct doesn't work for
		 * players, or under conflict or confusion. 
		 */
		if (!magr->mconf && !Conflict && otmp &&
		    mattk->aatyp != AT_WEAP && touch_petrifies(mdef->data)) {
		    strike = 0;
		    break;
		}
		dieroll = rnd(20 + i);
		if (!rn2(3) && magr->m_lev > 0) {
			magrlev = magr->m_lev;
			if (magrlev > 19) {
				magrhih = magrlev - 19;
				magrlev -= rnd(magrhih);

				if (magrlev > 29) {
					magrhih = magrlev - 19;
					if (magrhih > 1) magrhih /= 2;
					magrlev -= magrhih;
				}
			}
			tmp += rno(magrlev);
		}
		if (magr->data == &mons[PM_STOOGE_MOE] || magr->data == &mons[PM_STOOGE_CURLY] || magr->data == &mons[PM_STOOGE_LARRY]) tmp += 50;
		strike = (tmp > dieroll);
		if (strike) {
		    res[i] = hitmm(magr, mdef, mattk);
		    if((mdef->data == &mons[PM_BLACK_PUDDING] || mdef->data == &mons[PM_SHOCK_PUDDING] || mdef->data == &mons[PM_VOLT_PUDDING] || mdef->data == &mons[PM_DRUDDING] || mdef->data == &mons[PM_BLACK_DRUDDING] || mdef->data == &mons[PM_BLACKSTEEL_PUDDING] || mdef->data == &mons[PM_BLOOD_PUDDING] || mdef->data == &mons[PM_BLACK_PIERCER] || mdef->data == &mons[PM_BROWN_PUDDING])
		       && otmp && objects[otmp->otyp].oc_material == IRON
		       && mdef->mhp > 1 && !mdef->mcan && !rn2(100) ) /* slowing pudding farming to a crawl --Amy */
		    {
			if (clone_mon(mdef, 0, 0)) {
			    if (vis) {
				char buf[BUFSZ];

				strcpy(buf, Monnam(mdef));
				pline("%s divides as %s hits it!", buf, mon_nam(magr));
			    }
			}
		    }
		} else
		    missmm(magr, mdef, tmp, dieroll, mattk);
		/* KMH -- don't accumulate to-hit bonuses */
		if (otmp)
		    tmp -= hitval(otmp, mdef);
		break;

	    case AT_HUGS:	/* automatic if prev two attacks succeed, but also with a low chance otherwise --Amy */
		strike = ((i >= 2 && res[i-1] == MM_HIT && res[i-2] == MM_HIT) || !rn2(mdef->mtame ? 4 : 30));
		if (strike)
		    res[i] = hitmm(magr, mdef, mattk);

		break;

	    case AT_GAZE:
		strike = 0;	/* will not wake up a sleeper */
		res[i] = gazemm(magr, mdef, mattk);
		break;

	    case AT_EXPL:
		if (!magr->mtame && rn2(20)) break; /* we want the things to explode at YOU! Since monsters are immune to quite some attack types anyway, and the exploding lights would just suicide without causing any effect. --Amy */

		res[i] = explmm(magr, mdef, mattk);
		if (res[i] == MM_MISS) { /* cancelled--no attack */
		    strike = 0;
		    attk = 0;
		} else
		    strike = 1;	/* automatic hit */
		break;

	    case AT_ENGL:
		if (u.usteed && (mdef == u.usteed)) {
		    strike = 0;
		    break;
		} 
		/* Engulfing attacks are directed at the hero if
		 * possible. -dlc
		 */
		if (u.uswallow && magr == u.ustuck)
		    strike = 0;
		else {
		    if (!rn2(3) && magr->m_lev > 0) tmp += rno(magr->m_lev);

		    if ((strike = (tmp > (dieroll = rnd(20+i)))))
			res[i] = gulpmm(magr, mdef, mattk);
		    else
			missmm(magr, mdef, tmp, dieroll, mattk);
		}
		break;

	    default:		/* no attack */
		strike = 0;
		attk = 0;
		break;
	}

	boolean hashit = FALSE;

	if (attk && !(res[i] & MM_AGR_DIED)) {

	    res[i] = passivemm(magr, mdef, strike, res[i] & MM_DEF_DIED, 0);
	    if (res[i] & MM_HIT) hashit = TRUE;
	    if (hashit && !(res[i] & MM_HIT)) res[i] |= MM_HIT;
	    if (res[i] & MM_DEF_DIED) return res[i];

	    if (!(res[i] & MM_AGR_DIED) && !(res[i] & MM_DEF_DIED)) {
		    res[i] = passivemm(magr, mdef, strike, res[i] & MM_DEF_DIED, 1);
		    if (res[i] & MM_HIT) hashit = TRUE;
		    if (hashit && !(res[i] & MM_HIT)) res[i] |= MM_HIT;
		    if (res[i] & MM_DEF_DIED) return res[i];
	    }
	    if (!(res[i] & MM_AGR_DIED) && !(res[i] & MM_DEF_DIED)) {
		    res[i] = passivemm(magr, mdef, strike, res[i] & MM_DEF_DIED, 2);
		    if (res[i] & MM_HIT) hashit = TRUE;
		    if (hashit && !(res[i] & MM_HIT)) res[i] |= MM_HIT;
		    if (res[i] & MM_DEF_DIED) return res[i];
	    }
	    if (!(res[i] & MM_AGR_DIED) && !(res[i] & MM_DEF_DIED)) {
		    res[i] = passivemm(magr, mdef, strike, res[i] & MM_DEF_DIED, 3);
		    if (res[i] & MM_HIT) hashit = TRUE;
		    if (hashit && !(res[i] & MM_HIT)) res[i] |= MM_HIT;
		    if (res[i] & MM_DEF_DIED) return res[i];
	    }
	    if (!(res[i] & MM_AGR_DIED) && !(res[i] & MM_DEF_DIED)) {
		    res[i] = passivemm(magr, mdef, strike, res[i] & MM_DEF_DIED, 4);
		    if (res[i] & MM_HIT) hashit = TRUE;
		    if (hashit && !(res[i] & MM_HIT)) res[i] |= MM_HIT;
		    if (res[i] & MM_DEF_DIED) return res[i];
	    }
	    if (!(res[i] & MM_AGR_DIED) && !(res[i] & MM_DEF_DIED)) {
		    res[i] = passivemm(magr, mdef, strike, res[i] & MM_DEF_DIED, 5);
		    if (res[i] & MM_HIT) hashit = TRUE;
		    if (hashit && !(res[i] & MM_HIT)) res[i] |= MM_HIT;
		    if (res[i] & MM_DEF_DIED) return res[i];
	    }

	}

	if (hashit && !(res[i] & MM_HIT)) res[i] |= MM_HIT;

	if (res[i] & MM_DEF_DIED) return res[i];

	/*
	 *  Wake up the defender.  NOTE:  this must follow the check
	 *  to see if the defender died.  We don't want to modify
	 *  unallocated monsters!
	 */
	if (strike) mdef->msleeping = 0;

	if (res[i] & MM_AGR_DIED)  return res[i];
	/* return if aggressor can no longer attack */
	if (!magr->mcanmove || magr->msleeping) return res[i];
	if (res[i] & MM_HIT) struck = 1;	/* at least one hit */
    }

    /* egotypes and other extra attacks, by Amy */
    if (mdef->mtame) {

	if (magr->egotype_arcane ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SPEL;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_clerical ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CLRC;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_mastercaster ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CAST;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_thief ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SITM;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_disenchant ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_ENCH;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_rust ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_RUST;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_corrosion ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CORR;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_decay ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DCAY;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_wither ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_WTHR;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_grab ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_STCK;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_faker ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_FAKE;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_slows ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SLOW;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_vampire ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DRLI;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_teleportyou ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_TLPT;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_wrap ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_WRAP;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_disease ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DISE;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_slime ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SLIM;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_poisoner ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_POIS;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_elementalist ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_AXUS;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_acidspiller ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_ACID;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_engrave ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_NGRA;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_dark ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DARK;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_sounder ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SOUN;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_timer ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_TIME;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_thirster ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_THIR;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_nexus ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_NEXU;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_gravitator ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_GRAV;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_inert ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_INER;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_antimage ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_MANA;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_unskillor ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SKIL;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_venomizer ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_VENO;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_dreameater ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DREA;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_nastinator ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_NAST;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_baddie ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_BADE;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_sludgepuddle ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SLUD;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_vulnerator ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_VULN;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_marysue ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_FUMB;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_plasmon ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_PLAS;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_lasher ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_LASH;
		a->adtyp = AD_MALK;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_breather ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_RBRE;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_luck ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_LUCK;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_amnesiac ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_AMNE;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_seducer ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SSEX;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_cullen ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_VAMP;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_webber ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_WEBS;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_trapmaster ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_TRAP;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_itemporter ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_STTP;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_sinner  ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SIN;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_schizo ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DEPR;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_aligner) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_ALIN;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_destructor) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DEST;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_trembler) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_TREM;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_worldender) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_RAGN;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_damager) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_IDAM;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_antitype) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_ANTI;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_statdamager) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_STAT;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_damagedisher) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DAMA;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_sanitizer) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SANI;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_nastycurser) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_NACU;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_thiefguildmember) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_THIE;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_rogue) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SEDU;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_painlord) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_PAIN;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_empmaster) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_TECH;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_spellsucker) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_MEMO;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_eviltrainer) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_TRAI;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_contaminator ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CONT;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_reactor) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CONT;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_radiator) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CONT;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_minator) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_MINA;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_aggravator) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_AGGR;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_midiplayer ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_MIDI;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_rngabuser ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_RNG;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_watersplasher  ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = level.flags.lethe ? AD_LETH : AD_WET;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_cancellator ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CNCL;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_banisher ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_BANI;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_shredder ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SHRD;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_abductor ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_ABDC;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_incrementor ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CHKH;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_mirrorimage ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_HODS;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_curser ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CURS;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_horner ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = rn2(1000) ? AD_CHRN : AD_UVUU;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_push ) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DISP;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_randomizer) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_RBRE;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_blaster) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TENT;
		a->adtyp = AD_DRIN;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_psychic) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SPC2;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_abomination) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_SPC2;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->egotype_weeper) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_CONT;
		a->damn = 2;
		a->damd = (1 + (magr->m_lev));

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (evilfriday && magr->data->mlet == S_ZOMBIE) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_DISE;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (evilfriday && magr->data->mlet == S_MUMMY) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = rn2(20) ? AD_ICUR : AD_NACU;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (evilfriday && magr->data->mlet == S_GHOST) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_INER;
		a->damn = 1;
		a->damd = 1;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (uimplant && uimplant->oartifact == ART_POTATOROK && !rn2(10)) {

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) {
				ragnarok(FALSE);
				if (evilfriday && magr->m_lev > 1) evilragnarok(FALSE, magr->m_lev);
			}
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (uwep && uwep->oartifact == ART_RAFSCHAR_S_SUPERWEAPON && !rn2(10)) {

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) {
				ragnarok(FALSE);
				if (evilfriday && magr->m_lev > 1) evilragnarok(FALSE, magr->m_lev);
			}
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (uswapwep && uswapwep->oartifact == ART_RAFSCHAR_S_SUPERWEAPON && !rn2(10)) {

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) {
				ragnarok(FALSE);
				if (evilfriday && magr->m_lev > 1) evilragnarok(FALSE, magr->m_lev);
			}
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if (magr->data == &mons[PM_BOFH] && isevilvariant) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_PHYS;
		a->damn = 200;
		a->damd = 200;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) res[i] = hitmm(magr, mdef, a);
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

	if ((magr->data == &mons[PM_DNETHACK_ELDER_PRIEST_TM_] || magr->data == &mons[PM_SANDRA_S_MINDDRILL_SANDAL]) && isevilvariant) {

		mdat2 = &mons[PM_CAST_DUMMY];
		a = &mdat2->mattk[3];
		a->aatyp = AT_TUCH;
		a->adtyp = AD_PHYS;
		a->damn = 200;
		a->damd = 200;

		if(monnear(magr, mdef->mx, mdef->my)) {
			dieroll = rnd(20 + i);
			strike = (tmp > dieroll);
			if (strike) {
				res[i] = hitmm(magr, mdef, a);
				ragnarok(FALSE);
				if (evilfriday && magr->m_lev > 1) evilragnarok(FALSE, magr->m_lev);
			}
		}
		if (res[i] & MM_AGR_DIED) return res[i];
		if (res[i] & MM_DEF_DIED) return res[i];

	}

    } /* special attacks targetting pets */

    return(struck ? MM_HIT : MM_MISS);
}

/* monster attempts breath attack against another monster */
STATIC_OVL int
breamm(magr, mdef, mattk)
struct monst *magr, *mdef;
struct attack *mattk;
{
    /* if new breath types are added, change AD_ACID to max type */
    int typ = mattk->adtyp == AD_RBRE ? rnd(AD_SPC2) : mattk->adtyp;
    int mhp;

    if (linedup(mdef->mx, mdef->my, magr->mx, magr->my)) {
	if (magr->mcan) {
	    if (flags.soundok) {
		if (canseemon(magr))
		    pline("%s coughs.", Monnam(magr));
		else
		    You_hear("a cough.");
	    }
	} else if (!magr->mspec_used && rn2(3)) {
	    if (typ >= AD_MAGM && typ <= AD_SPC2) {
		if (canseemon(magr))
		    pline("%s breathes %s!", Monnam(magr), breathwep[typ-1]);
		mhp = mdef->mhp;
		buzz((int)(-20 - (typ-1)), (rn2(2) ? (int)mattk->damn : (int)mattk->damd ),
			magr->mx, magr->my, sgn(tbx), sgn(tby));
		nomul(0, 0, FALSE);
		/* breath runs out sometimes. */
		if (!rn2(3))
		    magr->mspec_used = 10+rn2(20);
		return (mdef->mhp < 1 ? MM_DEF_DIED : 0) |
		       (mdef->mhp < mhp ? MM_HIT : 0) |
		       (magr->mhp < 1 ? MM_AGR_DIED : 0);
	    } else impossible("Breath weapon %d used", typ-1);
	}
    }
    return MM_MISS;
}

/* monster attempts spit attack against another monster */
STATIC_OVL int
spitmm(magr, mdef, mattk)
struct monst *magr, *mdef;
struct attack *mattk;
{
    register struct obj *obj;
    int mhp;

    if (magr->mcan) {
	if (flags.soundok) {
	    if (canseemon(magr))
		pline("A dry rattle comes from %s throat.",
			s_suffix(mon_nam(magr)));
	    else
		You_hear("a dry rattle.");
	}
	return MM_MISS;
    }

    if (linedup(mdef->mx, mdef->my, magr->mx, magr->my)) {
	switch (mattk->adtyp) {
	    case AD_BLND:
	    case AD_DRST:
		obj = mksobj(BLINDING_VENOM, TRUE, FALSE);
		break;
	    case AD_DRLI:
		obj = mksobj(FAERIE_FLOSS_RHING, TRUE, FALSE);
		break;
	    case AD_TCKL:
		obj = mksobj(TAIL_SPIKES, TRUE, FALSE);
		break;
	    case AD_NAST:
		obj = mksobj(SEGFAULT_VENOM, TRUE, FALSE);
		break;
	    default:
		pline("bad attack type in spitmm");
	    /* fall through */
	    case AD_ACID:
		obj = mksobj(ACID_VENOM, TRUE, FALSE);
		break;
	}
	if (!obj) return MM_MISS;
	if (!rn2(BOLT_LIM - distmin(magr->mx, magr->my, mdef->mx, mdef->my))) {
	    if (canseemon(magr))
		pline("%s spits venom!", Monnam(magr));
	    mhp = mdef->mhp;
	    m_throw(magr, magr->mx, magr->my, sgn(tbx), sgn(tby),
		    distmin(magr->mx, magr->my, mdef->mx, mdef->my), obj);
	    nomul(0, 0, FALSE);
	    return (mdef->mhp < 1 ? MM_DEF_DIED : 0) |
		   (mdef->mhp < mhp ? MM_HIT : 0) |
		   (magr->mhp < 1 ? MM_AGR_DIED : 0);
	}
    }
    return MM_MISS;
}

/* monster attempts ranged weapon attack against another monster */
STATIC_OVL int
thrwmm(magr, mdef)
struct monst *magr, *mdef;
{
    struct obj *obj, *mwep;
    schar skill;
    int multishot, mhp;
    const char *onm;

	int polelimit = POLE_LIM;

    /* Rearranged beginning so monsters can use polearms not in a line */
    if (magr->weapon_check == NEED_WEAPON || !MON_WEP(magr)) {
	magr->weapon_check = NEED_RANGED_WEAPON;
	/* mon_wield_item resets weapon_check as appropriate */
	if(mon_wield_item(magr) != 0) return MM_MISS;
    }

    /* Pick a weapon */
    obj = select_rwep(magr,TRUE); /* can also select polearms even when far away from the player --Amy */
    if (!obj) return MM_MISS;

    if (is_applypole(obj)) {
	int dam, hitv, vis = canseemon(magr);

	if (obj->otyp == NOOB_POLLAX || obj->otyp == GREAT_POLLAX) polelimit += 5;
	if (obj->otyp == YITH_TENTACLE) polelimit += 2;
	if (obj->otyp == POLE_LANTERN) polelimit += 10;
	if (obj->otyp == NASTYPOLE) polelimit += 8;
	if (obj->oartifact == ART_ETHER_PENETRATOR) polelimit += 5;
	if (obj->oartifact == ART_FUURKER) polelimit += 6;
	if (obj->otyp == WOODEN_BAR) polelimit += 7;
	if (obj->oartifact == ART_OVERLONG_STICK) polelimit += 12;

	if (dist2(magr->mx, magr->my, mdef->mx, mdef->my) > polelimit ||
		!m_cansee(magr, mdef->mx, mdef->my))
	    return MM_MISS;	/* Out of range, or intervening wall */

	if (vis) {
	    onm = xname(obj);
	    pline("%s thrusts %s.", Monnam(magr),
		  obj_is_pname(obj) ? the(onm) : an(onm));
	}

	dam = dmgval(obj, mdef);
	hitv = 3 - distmin(mdef->mx, mdef->my, magr->mx, magr->my);
	if (hitv < -4) hitv = -4;
	if (bigmonst(mdef->data)) hitv++;
	hitv += 8 + obj->spe;
	if (mdef->mtame) hitv += magr->m_lev;
	if (dam < 1) dam = 1;

	if (find_mac(mdef) + hitv <= rnd(20)) {
	    if (flags.verbose && canseemon(mdef))
		pline("It misses %s.", mon_nam(mdef));
	    else if (vis)
		pline("It misses.");
	    return MM_MISS;
	} else {
	    if (flags.verbose && canseemon(mdef))
		pline("It hits %s%s", a_monnam(mdef), exclam(dam));
	    else if (vis)
		pline("It hits.");
	    if (objects[obj->otyp].oc_material == SILVER &&
		    hates_silver(mdef->data) && canseemon(mdef)) {
		if (vis)
		    pline_The("silver sears %s flesh!",
			    s_suffix(mon_nam(mdef)));
		else
		    pline("%s flesh is seared!", s_suffix(Monnam(mdef)));
	    }
	    if (objects[obj->otyp].oc_material == VIVA && hates_viva(mdef->data) && canseemon(mdef)) {
		    pline("%s is severely hurt by the radiation!", s_suffix(Monnam(mdef)));
	    }
	    if (objects[obj->otyp].oc_material == COPPER && hates_copper(mdef->data) && canseemon(mdef)) {
		    pline("%s decomposes from the contact with copper!", s_suffix(Monnam(mdef)));
	    }
	    if (obj->cursed && hates_cursed(mdef->data) && canseemon(mdef)) {
		    pline("%s is blasted by darkness!", s_suffix(Monnam(mdef)));
	    }
	    if (objects[obj->otyp].oc_material == INKA && hates_inka(mdef->data) && canseemon(mdef)) {
		    pline("%s is hurt by the inka string!", s_suffix(Monnam(mdef)));
	    }
	    if (obj->otyp == ODOR_SHOT && hates_odor(mdef->data) && canseemon(mdef)) {
		    pline("%s groans at the odor!", s_suffix(Monnam(mdef)));
	    }
	    mdef->mhp -= dam;
	    if (mdef->mhp < 1) {
		if (canseemon(mdef))
		    pline("%s is %s!", Monnam(mdef),
			    (nonliving(mdef->data) || !canspotmon(mdef))
			    ? "destroyed" : "killed");
		mondied(mdef);
		return MM_DEF_DIED | MM_HIT;
	    }
	    else
		return MM_HIT;
	}
    }

    if (!linedup(mdef->mx, mdef->my, magr->mx, magr->my))
	return MM_MISS;

    skill = objects[obj->otyp].oc_skill;
    mwep = MON_WEP(magr);		/* wielded weapon */

    if (mwep && ammo_and_launcher(obj, mwep) && objects[mwep->otyp].oc_range &&
	    dist2(magr->mx, magr->my, mdef->mx, mdef->my) >
	    objects[mwep->otyp].oc_range * objects[mwep->otyp].oc_range)
	return MM_MISS; /* Out of range */

    /* Multishot calculations */
    multishot = 1;
    if (( (mwep && ammo_and_launcher(obj, mwep)) || skill == P_DAGGER ||
	    skill == -P_DART || skill == -P_SHURIKEN) && !magr->mconf) {
	/* Assumes lords are skilled, princes are expert */
	if (is_prince(magr->data)) multishot += 2;
	else if (is_lord(magr->data)) multishot++;

	/* strong, nasty or high-level monsters can also shoot more --Amy */
	if (magr->m_lev >= 10 && strongmonst(magr->data) && !rn2(3)) multishot++;
	if (magr->m_lev >= 10 && strongmonst(magr->data) && !rn2(9)) multishot++;
	if (magr->m_lev >= 10 && strongmonst(magr->data) && !rn2(27)) multishot++;

	if (magr->m_lev >= 10 && extra_nasty(magr->data) && !rn2(2)) multishot++;
	if (magr->m_lev >= 10 && extra_nasty(magr->data) && !rn2(4)) multishot++;
	if (magr->m_lev >= 10 && extra_nasty(magr->data) && !rn2(8)) multishot++;

	if (magr->m_lev >= 10 && magr->m_lev < 20) multishot += 1;
	if (magr->m_lev >= 20 && magr->m_lev < 30) multishot += rnd(2);
	if (magr->m_lev >= 30 && magr->m_lev < 40) multishot += rnd(3);
	if (magr->m_lev >= 40 && magr->m_lev < 50) multishot += rnd(4);
	if (magr->m_lev >= 50 && magr->m_lev < 60) multishot += rnd(5);
	if (magr->m_lev >= 60 && magr->m_lev < 70) multishot += rnd(6);
	if (magr->m_lev >= 70 && magr->m_lev < 80) multishot += rnd(7);
	if (magr->m_lev >= 80 && magr->m_lev < 90) multishot += rnd(8);
	if (magr->m_lev >= 90 && magr->m_lev < 100) multishot += rnd(9);
	if (magr->m_lev >= 100) multishot += rnd(10);

	/*  Elven Craftsmanship makes for light,  quick bows */
	if (obj->otyp == ELVEN_ARROW && !obj->cursed)
	    multishot++;
	if (mwep && mwep->otyp == ELVEN_BOW && !mwep->cursed) multishot++;

	if (mwep && mwep->otyp == WILDHILD_BOW && obj->otyp == ODOR_SHOT) multishot++;
	if (mwep && mwep->otyp == COMPOST_BOW && obj->otyp == FORBIDDEN_ARROW) multishot++;

	if (mwep && mwep->otyp == CATAPULT) multishot += rnd(5);

	if (mwep && mwep->otyp == HYDRA_BOW) multishot += 2;
	if (mwep && mwep->otyp == WILDHILD_BOW) multishot += 2;

	/* 1/3 of object enchantment */
	if (mwep && mwep->spe > 1)
	    multishot += rounddiv(mwep->spe, 3);
	/* Some randomness */
	if (multishot > 1)
	    multishot = rnd(multishot);
	if (mwep && objects[mwep->otyp].oc_rof && is_launcher(mwep))
	    multishot += objects[mwep->otyp].oc_rof;

	switch (monsndx(magr->data)) {
	case PM_SPARD:
		multishot += 3;
		break;
	case PM_RANGER:
	case PM_ROCKER:
	case PM_GATLING_ARCHER:
		multishot++;
		break;
	case PM_PELLET_ARCHER:
	case PM_ECM_ARCHER:
	case PM_SHOTGUN_HORROR:
	case PM_SHOTGUN_TERROR:
	case PM_KOBOLD_PEPPERMASTER:
		multishot++;
		multishot++;
		break;
	case PM_BRA_GIANT:
		multishot += 5;
		break;
	case PM_ELPH:
		multishot++;
		if (obj->otyp == ELVEN_ARROW && mwep && mwep->otyp == ELVEN_BOW) multishot++;
		break;
	case PM_ROGUE:
		if (skill == P_DAGGER) multishot++;
		break;
	case PM_NINJA_GAIDEN:
	case PM_NINJA:
	case PM_SAMURAI:
		if (obj->otyp == YA && mwep &&
		    mwep->otyp == YUMI) multishot++;
		break;
	default:
	    break;
	}
	/* racial bonus */
	if ((is_elf(magr->data) &&
		obj->otyp == ELVEN_ARROW &&
		mwep && mwep->otyp == ELVEN_BOW) ||
	    (is_orc(magr->data) &&
		obj->otyp == ORCISH_ARROW &&
		mwep && mwep->otyp == ORCISH_BOW))
	    multishot++;

	/* monster-versus-monster is less critical than monster-versus-player, so we don't put the reduction for
	 * weaker monsters here that is present in mthrowu.c --Amy */

	if ((long)multishot > obj->quan) multishot = (int)obj->quan;
	if (multishot < 1) multishot = 1;
	/* else multishot = rnd(multishot); */
    }

    if (canseemon(magr)) {
	char onmbuf[BUFSZ];

	if (multishot > 1) {
	    /* "N arrows"; multishot > 1 implies obj->quan > 1, so
	       xname()'s result will already be pluralized */
	    sprintf(onmbuf, "%d %s", multishot, xname(obj));
	    onm = onmbuf;
	} else {
	    /* "an arrow" */
	    onm = singular(obj, xname);
	    onm = obj_is_pname(obj) ? the(onm) : an(onm);
	}
	m_shot.s = (mwep && ammo_and_launcher(obj,mwep)) ? TRUE : FALSE;
	pline("%s %s %s!", Monnam(magr),
	      m_shot.s ? is_bullet(obj) ? "fires" : "shoots" : "throws",
	      onm);
	m_shot.o = obj->otyp;
    } else {
	m_shot.o = STRANGE_OBJECT;	/* don't give multishot feedback */
    }

    mhp = mdef->mhp;
    m_shot.n = multishot;
    for (m_shot.i = 1; m_shot.i <= m_shot.n; m_shot.i++)
	m_throw(magr, magr->mx, magr->my, sgn(tbx), sgn(tby),
		distmin(magr->mx, magr->my, mdef->mx, mdef->my), obj);
    m_shot.n = m_shot.i = 0;
    m_shot.o = STRANGE_OBJECT;
    m_shot.s = FALSE;

    nomul(0, 0, FALSE);

    return (mdef->mhp < 1 ? MM_DEF_DIED : 0) | (mdef->mhp < mhp ? MM_HIT : 0) |
	   (magr->mhp < 1 ? MM_AGR_DIED : 0);
}

/* Returns the result of mdamagem(). */
STATIC_OVL int
hitmm(magr, mdef, mattk)
	register struct monst *magr,*mdef;
	struct	attack *mattk;
{
	if(vis){
		int compat;
		char buf[BUFSZ], mdef_name[BUFSZ];

		if (!canspotmon(magr))
		    map_invisible(magr->mx, magr->my);
		if (!canspotmon(mdef))
		    map_invisible(mdef->mx, mdef->my);
		if(mdef->m_ap_type) seemimic(mdef);
		if(magr->m_ap_type) seemimic(magr);
		if((compat = could_seduce(magr,mdef,mattk)) && !magr->mcan) {
			sprintf(buf, "%s %s", Monnam(magr),
				mdef->mcansee ? "smiles at" : "talks to");
			pline("%s %s %s.", buf, mon_nam(mdef),
				compat == 2 ?
					"engagingly" : "seductively");
		} else {
		    char magr_name[BUFSZ];

		    strcpy(magr_name, Monnam(magr));
		    switch (mattk->aatyp) {
			case AT_BITE:
				sprintf(buf,"%s bites", magr_name);
				break;
			case AT_CLAW:
				sprintf(buf,"%s claws", magr_name);
				break;
			case AT_STNG:
				sprintf(buf,"%s stings", magr_name);
				break;
			case AT_BUTT:
				sprintf(buf,"%s butts", magr_name);
				break;
			case AT_LASH:
				sprintf(buf,"%s lashes", magr_name);
				break;
			case AT_TRAM:
				sprintf(buf,"%s tramples over", magr_name);
				break;
			case AT_SCRA:
				sprintf(buf,"%s scratches", magr_name);
				break;
			case AT_TUCH:
				sprintf(buf,"%s touches", magr_name);
				break;
			case AT_BEAM:
				sprintf(buf,"%s blasts", magr_name);
				break;
			case AT_BREA:
				sprintf(buf,"%s breathes at", magr_name);
				break;
			case AT_SPIT:
				sprintf(buf,"%s spits at", magr_name);
				break;
			case AT_TENT:
				sprintf(buf, "%s tentacles suck",
					s_suffix(magr_name));
				break;
			case AT_HUGS:
				if (magr != u.ustuck) {
				    sprintf(buf,"%s squeezes", magr_name);
				    break;
				}
			case AT_MULTIPLY:
				/* No message. */
				break;
			default:
				sprintf(buf,"%s hits", magr_name);
		    }
		    pline("%s %s.", buf, mon_nam_too(mdef_name, mdef, magr));
		}
	} else /* not vis */  noises(magr, mattk);

	/* stooges infighting but not actually hurting each other, ported from nethack 2.3e by Amy */
	if ((magr->data == &mons[PM_STOOGE_LARRY] || magr->data == &mons[PM_STOOGE_CURLY] || magr->data == &mons[PM_STOOGE_MOE]) && (mdef->data == &mons[PM_STOOGE_LARRY] || mdef->data == &mons[PM_STOOGE_CURLY] || mdef->data == &mons[PM_STOOGE_MOE])) {

		if (!rn2(6) && !mdef->mblinded && mdef->mcansee) {
			if(vis) pline("%s is poked in the %s!", Monnam(mdef), mbodypart(mdef, EYE));
			mdef->mcansee = 0;
			mdef->mblinded += rnd(10);
			if (mdef->mblinded <= 0) mdef->mblinded = 127;
		} else if (vis) {
			switch (rn2(100)) {
			case 0 : pline("%s is shoved!", Monnam(mdef)); 
				break;
			case 1 : pline("%s is kicked!", Monnam(mdef));
				break;
			case 2 : pline("%s is slapped!", Monnam(mdef));
				break;
			case 3 : pline("%s is slugged!", Monnam(mdef));
				break;
			case 4 : pline("%s is punched!", Monnam(mdef));
				break;
			case 5 : pline("%s is pinched!", Monnam(mdef));
				break;
			case 6 : pline("But %s dodges!", mon_nam(mdef));
				break;
			case 7 : pline("But %s ducks!", mon_nam(mdef));
				break;
			case 8 : pline("%s gets a black %s!", Monnam(mdef), mbodypart(mdef, EYE));
				break;
			case 9 : pline("%s gets a bloody %s!", Monnam(mdef), mbodypart(mdef, NOSE));
				break;
			case 10: pline("%s gets a broken tooth!", Monnam(mdef));
				break;
			default: break; /* nothing */
			}
		}
		if (!rn2(2))
			stoogejoke();

		return 0;
	}

	return(mdamagem(magr, mdef, mattk));
}

/* Returns the same values as mdamagem(). */
STATIC_OVL int
gazemm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	struct attack *mattk;
{
	char buf[BUFSZ];

	if(vis) {
		sprintf(buf,"%s gazes at", Monnam(magr));
		pline("%s %s...", buf, mon_nam(mdef));
	}

	if (magr->mcan || !magr->mcansee || magr->minvisreal ||
	    (magr->minvis && !perceives(mdef->data)) ||
	    !mdef->mcansee || mdef->msleeping) {
	    if(vis) pline("but nothing happens.");
	    return(MM_MISS);
	}
	/* call mon_reflects 2x, first test, then, if visible, print message */
	if (magr->data == &mons[PM_MEDUSA] && mon_reflects(mdef, (char *)0)) {
	    if (canseemon(mdef))
		(void) mon_reflects(mdef,
				    "The gaze is reflected away by %s %s.");
	    if (mdef->mcansee) {
		if (mon_reflects(magr, (char *)0)) {
		    if (canseemon(magr))
			(void) mon_reflects(magr,
					"The gaze is reflected away by %s %s.");
		    return (MM_MISS);
		}
		if ((mdef->minvis && !perceives(magr->data)) || mdef->minvisreal) {
		    if (canseemon(magr)) {
			pline("%s doesn't seem to notice that %s gaze was reflected.",
			      Monnam(magr), mhis(magr));
		    }
		    return (MM_MISS);
		}
		if (canseemon(magr))
		    pline("%s is turned to stone!", Monnam(magr));
		monstone(magr);
		if (magr->mhp > 0) return (MM_MISS);
		return (MM_AGR_DIED);
	    }
	}

	return(mdamagem(magr, mdef, mattk));
}

/* Returns the same values as mattackm(). */
STATIC_OVL int
gulpmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	register struct	attack *mattk;
{
	xchar	ax, ay, dx, dy;
	int	status;
	char buf[BUFSZ];
	struct obj *obj;

	if (mdef->data->msize >= MZ_HUGE) return MM_MISS;

	if (vis) {
		sprintf(buf,"%s swallows", Monnam(magr));
		pline("%s %s.", buf, mon_nam(mdef));
	}
	for (obj = mdef->minvent; obj; obj = obj->nobj)
	    (void) snuff_lit(obj);

	/*
	 *  All of this maniuplation is needed to keep the display correct.
	 *  There is a flush at the next pline().
	 */
	ax = magr->mx;
	ay = magr->my;
	dx = mdef->mx;
	dy = mdef->my;
	/*
	 *  Leave the defender in the monster chain at it's current position,
	 *  but don't leave it on the screen.  Move the agressor to the def-
	 *  ender's position.
	 */
	remove_monster(ax, ay);
	place_monster(magr, dx, dy);
	newsym(ax,ay);			/* erase old position */
	newsym(dx,dy);			/* update new position */

	status = mdamagem(magr, mdef, mattk);

	if ((status & MM_AGR_DIED) && (status & MM_DEF_DIED)) {
	    ;					/* both died -- do nothing  */
	}
	else if (status & MM_DEF_DIED) {	/* defender died */
	    /*
	     *  Note:  remove_monster() was called in relmon(), wiping out
	     *  magr from level.monsters[mdef->mx][mdef->my].  We need to
	     *  put it back and display it.	-kd
	     */
	    place_monster(magr, dx, dy);
	    newsym(dx, dy);
	}
	else if (status & MM_AGR_DIED) {	/* agressor died */
	    place_monster(mdef, dx, dy);
	    newsym(dx, dy);
	}
	else {					/* both alive, put them back */
	    if (cansee(dx, dy))
		pline("%s is regurgitated!", Monnam(mdef));

	    place_monster(magr, ax, ay);
	    place_monster(mdef, dx, dy);
	    newsym(ax, ay);
	    newsym(dx, dy);
	}

	return status;
}

STATIC_OVL int
explmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	register struct	attack *mattk;
{
	int result;

	if (magr->mcan)
	    return MM_MISS;

	if(cansee(magr->mx, magr->my))
		pline("%s explodes!", Monnam(magr));
	else	noises(magr, mattk);

	remove_monster(magr->mx, magr->my);     /* MAR */
	result = mdamagem(magr, mdef, mattk);
	place_monster(magr,magr->mx, magr->my); /* MAR */

	/* Kill off agressor if it didn't die. */
	if (!(result & MM_AGR_DIED)) {
	    mondead(magr);
	    if (magr->mhp > 0) return result;	/* life saved */
	    result |= MM_AGR_DIED;
	}
	/* KMH -- Player gets blame for flame/freezing sphere */
	if (magr->isspell && !(result & MM_DEF_DIED))
		setmangry(mdef);
	/* give this one even if it was visible, except for spell creatures */
	if (magr->mtame && !magr->isspell)
	    You(brief_feeling, "melancholy");

	return result;
}

/*
 *  See comment at top of mattackm(), for return values.
 */
STATIC_OVL int
mdamagem(magr, mdef, mattk)
	register struct monst	*magr, *mdef;
	register struct attack	*mattk;
{
	struct obj *obj;
	char buf[BUFSZ];
	struct permonst *pa = magr->data, *pd = mdef->data;
	int armpro, num, tmp = d((int)mattk->damn, (int)mattk->damd);
	boolean cancelled;
	int canhitmon, objenchant;        
        boolean nohit = FALSE;

	int petdamagebonus;
	int atttyp;

	if (touch_petrifies(pd) && !rn2(4) && !resists_ston(magr)) {
	    long protector = attk_protection((int)mattk->aatyp),
		 wornitems = magr->misc_worn_check;

	    /* wielded weapon gives same protection as gloves here */
	    if (otmp != 0) wornitems |= W_ARMG;

	    if (protector == 0L ||
		  (protector != ~0L && (wornitems & protector) != protector)) {
		if (poly_when_stoned(pa)) {
		    mon_to_stone(magr);
		    return MM_HIT; /* no damage during the polymorph */
		}
		if (vis) pline("%s turns to stone!", Monnam(magr));
		monstone(magr);
		if (magr->mhp > 0) return 0;
		else if (magr->mtame && !vis)
		    You(brief_feeling, "peculiarly sad");
		return MM_AGR_DIED;
	    }
	}

	canhitmon = 0;
	if (need_one(mdef))    canhitmon = 1;
	if (need_two(mdef))    canhitmon = 2;
	if (need_three(mdef))  canhitmon = 3;
	if (need_four(mdef))   canhitmon = 4;

	if (mattk->aatyp == AT_WEAP && otmp) {
	    objenchant = otmp->spe;
	    if (objenchant < 0) objenchant = 0;
	    if (otmp->oartifact) {
		if (otmp->spe < 2) objenchant += 1;
		else objenchant = 2;
	    }
	    if (is_lightsaber(otmp)) objenchant = 4;
	} else objenchant = 0;

	/* a monster that needs a +1 weapon to hit it hits as a +1 weapon... */
	if (need_one(magr))    objenchant = 1;
	if (need_two(magr))    objenchant = 2;
	if (need_three(magr))  objenchant = 3;
	if (need_four(magr))   objenchant = 4;
	/* overridden by specific flags */
	if (hit_as_one(magr))    objenchant = 1;
	if (hit_as_two(magr))    objenchant = 2;
	if (hit_as_three(magr))  objenchant = 3;
	if (hit_as_four(magr))   objenchant = 4;

	if (objenchant < canhitmon && !rn2(3)) nohit = TRUE;

	/* cancellation factor is the same as when attacking the hero */
	armpro = magic_negation(mdef);
	if (mdef->data->mr >= 49) armpro++; /* highly magic resistant monsters should have magic cancellation --Amy */
	if (mdef->data->mr >= 69) armpro++;
	if (mdef->data->mr >= 99) armpro++;
	cancelled = magr->mcan || !((rn2(3) >= armpro) || !rn2(50));

	petdamagebonus = 100;

	if (magr->mtame && !mdef->mtame && !PlayerCannotUseSkills) { /* bonus damage to make pets more viable --Amy */
		switch (P_SKILL(P_PETKEEPING)) {

	      	case P_BASIC:	petdamagebonus += 16; break;
	      	case P_SKILLED:	petdamagebonus += 32; break;
	      	case P_EXPERT:	petdamagebonus += 50; break;
	      	case P_MASTER:	petdamagebonus += 75; break;
	      	case P_GRAND_MASTER:	petdamagebonus += 100; break;
	      	case P_SUPREME_MASTER:	petdamagebonus += 150; break;
			default: break;
		
		}

	}

	if (magr->mtame && !mdef->mtame && (magr->data->mlet == S_QUADRUPED) && Race_if(PM_ENGCHIP)) {
		petdamagebonus += 25;
	}

	if (magr->mtame && !mdef->mtame) {
		/* and a little help if pet's experience level is very high, to make large cats etc. more useful --Amy */
		int overlevelled = 0;
		if (magr->m_lev > magr->data->mlevel) overlevelled = ((magr->m_lev - magr->data->mlevel) * 3 / 2);
		if (overlevelled > 0) {
			petdamagebonus += overlevelled;
		}

		/* it is not a bug that uhitm.c multiplies the level difference by two and this function only gives a
		 * 50% boost, because your max level is only 30, while pets can reach 49 --Amy */

	}

	/* riding skill is now finally useful too, as it boosts steed damage --Amy */
	if (u.usteed && magr == u.usteed && !mdef->mtame && !PlayerCannotUseSkills) {
		switch (P_SKILL(P_RIDING)) {

	      	case P_BASIC:	petdamagebonus += 16; break;
	      	case P_SKILLED:	petdamagebonus += 32; break;
	      	case P_EXPERT:	petdamagebonus += 50; break;
	      	case P_MASTER:	petdamagebonus += 75; break;
	      	case P_GRAND_MASTER:	petdamagebonus += 100; break;
	      	case P_SUPREME_MASTER:	petdamagebonus += 150; break;
			default: break;
		
		}

	}

	if (magr->egotype_champion) petdamagebonus += 10; /* smaller bonuses than mhitu, intentional --Amy */
	if (magr->egotype_boss) petdamagebonus += 25;
	if (magr->egotype_atomizer) petdamagebonus += 50;

	/* tame bosses are simply better --Amy */
	if (magr->mtame && !mdef->mtame && (magr->data->geno & G_UNIQ)) petdamagebonus += 25;

	if (petdamagebonus > 100 && (tmp > 1 || (tmp == 1 && petdamagebonus >= 150) )) {

		tmp *= petdamagebonus;
		tmp /= 100;

	}

	if (mdef->mtame) {
		if (magr->egotype_champion) {
			tmp *= 110;
			tmp /= 100;
		}
		if (magr->egotype_boss) {
			tmp *= 125;
			tmp /= 100;
		}
		if (magr->data->geno & G_UNIQ) {
			tmp *= 125;
			tmp /= 100;
		}
		if (magr->egotype_atomizer) {
			tmp *= 150;
			tmp /= 100;
		}

	}

	atttyp = mattk->adtyp;

	if (mdef->mtame) {
		if (atttyp == AD_RBRE) {
			while (atttyp == AD_ENDS || atttyp == AD_RBRE || atttyp == AD_WERE) {
				atttyp = randattack();
			}
		}

		if (atttyp == AD_DAMA) {
			atttyp = randomdamageattack();
		}

		if (atttyp == AD_THIE) {
			atttyp = randomthievingattack();
		}

		if (atttyp == AD_RNG) {
			while (atttyp == AD_ENDS || atttyp == AD_RNG || atttyp == AD_WERE) {
				atttyp = rn2(AD_ENDS); }
		}

		if (atttyp == AD_PART) atttyp = u.adpartattack;

		if (atttyp == AD_MIDI) {
			atttyp = magr->m_id;
			if (atttyp < 0) atttyp *= -1;
			while (atttyp >= AD_ENDS) atttyp -= AD_ENDS;
			if (!(atttyp >= AD_PHYS && atttyp < AD_ENDS)) atttyp = AD_PHYS; /* fail safe --Amy */
			if (atttyp == AD_WERE) atttyp = AD_PHYS;
		}
	}

	switch(atttyp) {
	    case AD_DGST:

          if (!rnd(25)) { /* since this is an instakill, greatly lower the chance of it connecting --Amy */
		if (nohit) nohit = FALSE;                
		/* eating a Rider or its corpse is fatal */
		if (is_rider(mdef->data) || is_deadlysin(mdef->data) ) {
		    if (vis)
			pline("%s %s!", Monnam(magr),
			      mdef->data == &mons[PM_FAMINE] ?
				"belches feebly, shrivels up and dies" :
			      mdef->data == &mons[PM_PESTILENCE] ?
				"coughs spasmodically and collapses" :
				"vomits violently and drops dead");
		    mondied(magr);
		    if (magr->mhp > 0) return 0;	/* lifesaved */
		    else if (magr->mtame && !vis)
			You(brief_feeling, "queasy");
		    return MM_AGR_DIED;
		}
		if(flags.verbose && flags.soundok) verbalize("Burrrrp!");
		tmp = mdef->mhp;
		/* Use up amulet of life saving */
		if (!!(obj = mlifesaver(mdef))) m_useup(mdef, obj);

		/* Is a corpse for nutrition possible?  It may kill magr */
		if (!corpse_chance(mdef, magr, TRUE) || magr->mhp < 1)
		    break;

		/* Pets get nutrition from swallowing monster whole.
		 * No nutrition from G_NOCORPSE monster, eg, undead.
		 * DGST monsters don't die from undead corpses
		 */
		num = monsndx(mdef->data);
		if (magr->mtame && !magr->isminion &&
		    !(mvitals[num].mvflags & G_NOCORPSE)) {
		    struct obj *virtualcorpse = mksobj(CORPSE, FALSE, FALSE);
		    int nutrit;

		    if (virtualcorpse) {

			    virtualcorpse->corpsenm = num;
			    virtualcorpse->owt = weight(virtualcorpse);
			    nutrit = dog_nutrition(magr, virtualcorpse);
			    dealloc_obj(virtualcorpse);

		    }

		    /* only 50% nutrition, 25% of normal eating time */
		    if (magr->meating > 1) magr->meating = (magr->meating+3)/4;
		    if (nutrit > 1) nutrit /= 2;
		    EDOG(magr)->hungrytime += nutrit;
		}
          }
		break;
	    case AD_STUN:
	    case AD_FUMB:
	    case AD_TREM:
	    case AD_SOUN:
		if (magr->mcan) break;
		if (canseemon(mdef))
		    pline("%s %s for a moment.", Monnam(mdef),
			  makeplural(stagger(mdef->data, "stagger")));
		mdef->mstun = 1;
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		goto physical;
	    case AD_LEGS:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		goto physical;
	    case AD_WERE:
	    case AD_HEAL:
	    case AD_PHYS:
physical:
		if (mattk->aatyp == AT_WEAP && otmp) {
		    if (otmp->otyp == CORPSE &&
			    touch_petrifies(&mons[otmp->corpsenm]) && nohit)
			nohit = FALSE;
		} else if(nohit) break;                
		if (mattk->aatyp == AT_KICK && thick_skinned(pd) && tmp) {
		    tmp = 1;
		} else if(mattk->aatyp == AT_WEAP) {
		    if(otmp) {
			if (otmp->otyp == CORPSE &&
				touch_petrifies(&mons[otmp->corpsenm]))
			    goto do_stone;

			/* WAC -- Real weapon?
			 * Could be stuck with a cursed bow/polearm it wielded
			 */
			if (/* if you strike with a bow... */
			    is_launcher(otmp) ||
			    /* or strike with a missile in your hand... */
			    (is_missile(otmp) || is_ammo(otmp)) ||
			    /* lightsaber that isn't lit ;) */
			    (is_lightsaber(otmp) && !otmp->lamplit) ||
			    /* WAC -- or using a pole at short range... */
			    (is_pole(otmp))) {
			    /* then do only 1-2 points of damage */
			    if ( (is_shade(pd) || (mdef && mdef->egotype_shader)) && objects[otmp->otyp].oc_material != SILVER && objects[otmp->otyp].oc_material != ARCANIUM)
				tmp = 0;
			    else
				tmp = rnd(2);

#if 0 /* Monsters don't wield boomerangs */
		    	    if(otmp->otyp == BOOMERANG /* && !rnl(3) */) {
				pline("As %s hits you, %s breaks into splinters.",
				      mon_nam(mtmp), the(xname(otmp)));
				useup(otmp);
				otmp = (struct obj *) 0;
				possibly_unwield(mtmp);
				if (!is_shade(pd) && !(mdef && mdef->egotype_shader) )
				    tmp++;
		    	    }
#endif			
			} else tmp += dmgval(otmp, mdef);

			/* MRKR: Handling damage when hitting with */
			/*       a burning torch */

			if(otmp->otyp == TORCH && otmp->lamplit
			   && !resists_fire(mdef)) {

			  if (!Blind) {
			    static char outbuf[BUFSZ];
			    char *s = Shk_Your(outbuf, otmp);

			    boolean water = (mdef->data ==
					     &mons[PM_WATER_ELEMENTAL]);

			    pline("%s %s %s%s %s%s.", s, xname(otmp),
				  (water ? "vaporize" : "burn"),
				  (otmp->quan > 1L ? "" : "s"),
				  (water ? "part of " : ""), mon_nam(mdef));
			  }

			  burn_faster(otmp, 1);

			  tmp++;
			  if (resists_cold(mdef)) tmp += rnd(3);

			  if (!rn2(33)) burnarmor(mdef);
			    if (!rn2(33))
			      (void)destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
			    if (!rn2(33))
			      (void)destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
			    if (!rn2(50))
			      (void)destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);

			}

                        /* WAC Weres get seared */
                        if(otmp && objects[otmp->otyp].oc_material == SILVER && (hates_silver(pd))) {
                                tmp += 8;
                                if (vis) pline("The silver sears %s!", mon_nam(mdef));
                        }
                        if(otmp && objects[otmp->otyp].oc_material == COPPER && (hates_copper(pd))) { 
                                tmp += 20;
                                if (vis) pline("The copper decomposes %s!", mon_nam(mdef));
                        }
                        if(otmp && otmp->cursed && (hates_cursed(pd))) { 
                                tmp += 4;
					  if (otmp->hvycurse) tmp += 4;
					  if (otmp->prmcurse) tmp += 7;
					  if (otmp->bbrcurse) tmp += 15;
					  if (otmp->evilcurse) tmp += 15;
					  if (otmp->morgcurse) tmp += 15;
                                if (vis) pline("The unholy aura blasts %s!", mon_nam(mdef));
                        }
                        if(otmp && objects[otmp->otyp].oc_material == VIVA && (hates_viva(pd))) { 
                                tmp += 20;
                                if (vis) pline("The radiation damages %s!", mon_nam(mdef));
                        }
                        if(otmp && objects[otmp->otyp].oc_material == INKA && (hates_inka(pd))) { 
                                tmp += 5;
                                if (vis) pline("The inka string damages %s!", mon_nam(mdef));
                        }
                        if(otmp && otmp->otyp == ODOR_SHOT && (hates_odor(pd))) { 
                                tmp += rnd(10);
                                if (vis) pline("The odor beguils %s!", mon_nam(mdef));
                        }
                        /* Stakes do extra dmg agains vamps */
                        if (otmp && (otmp->otyp == WOODEN_STAKE || otmp->oartifact == ART_VAMPIRE_KILLER) && is_vampire(pd)) {
                                if(otmp->oartifact == ART_STAKE_OF_VAN_HELSING) {
                                        if (!rn2(10)) {
                                                if (vis) {
                                                        strcpy(buf, Monnam(magr));
                                                        pline("%s plunges the stake into the heart of %s.",
                                                                buf, mon_nam(mdef));
                                                        pline("%s's body vaporizes!", Monnam(mdef));
                                                }
                                                mondead(mdef); /* no corpse */
                                                if (mdef->mhp < 0) return (MM_DEF_DIED |
                                                        (grow_up(magr,mdef) ? 0 : MM_AGR_DIED));                                                
                                        } else {
                                                if (vis) {
                                                        strcpy(buf, Monnam(magr));
                                                        pline("%s drives the stake into %s.",
                                                                buf, mon_nam(mdef));
                                                }
                                                tmp += rnd(6) + 2;
                                        }
                                }else if (otmp->oartifact == ART_VAMPIRE_KILLER) {
                                        if (vis) {
                                                strcpy(buf, Monnam(magr));
                                                pline("%s whips %s good!",
                                                        buf, mon_nam(mdef));
                                        }
                                        tmp += rnd(6);
                                }
					 else {
                                        if (vis) {
                                                strcpy(buf, Monnam(magr));
                                                pline("%s drives the stake into %s.",
                                                        buf, mon_nam(mdef));
                                        }
                                        tmp += rnd(6);
                                }
                        }

                        if (otmp && otmp->oartifact) {
			    (void)artifact_hit(magr,mdef, otmp, &tmp, dieroll);
			    if (mdef->mhp <= 0)
				return (MM_DEF_DIED |
					(grow_up(magr,mdef) ? 0 : MM_AGR_DIED));
			}
			if (otmp && tmp)
				mrustm(magr, mdef, otmp);
		    }
		} else if (magr->data == &mons[PM_PURPLE_WORM] &&
			    mdef->data == &mons[PM_SHRIEKER]) {
		    /* hack to enhance mm_aggression(); we don't want purple
		       worm's bite attack to kill a shrieker because then it
		       won't swallow the corpse; but if the target survives,
		       the subsequent engulf attack should accomplish that */
		    if (tmp >= mdef->mhp) tmp = mdef->mhp - 1;
		}
		break;
	    case AD_FIRE:
		if (nohit) break;
		
		if (cancelled) {
		    tmp = 0;
		    break;
		}
		if (vis)
		    pline("%s is %s!", Monnam(mdef),
			  on_fire(mdef->data, mattk));
		if (pd == &mons[PM_STRAW_GOLEM] ||
		    pd == &mons[PM_WAX_GOLEM] ||
		    pd == &mons[PM_PAPER_GOLEM]) {
			if (vis) pline("%s burns completely!", Monnam(mdef));
			mondied(mdef);
			if (mdef->mhp > 0) return 0;
			else if (mdef->mtame && !vis)
			    pline("May %s roast in peace.", mon_nam(mdef));
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		if (!rn2(33)) tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
		if (!rn2(33)) tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
		if (resists_fire(mdef)) {
		    if (vis)
			pline_The("fire doesn't seem to burn %s!",
								mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_FIRE, tmp);
		    tmp = 0;
		}
		/* only potions damage resistant players in destroy_item */
		if (!rn2(33)) tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
		break;
	    case AD_COLD:
		if (nohit) break;
		
		if (cancelled) {
		    tmp = 0;
		    break;
		}
		if (vis) pline("%s is covered in frost!", Monnam(mdef));
		if (resists_cold(mdef)) {
		    if (vis)
			pline_The("frost doesn't seem to chill %s!",
								mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_COLD, tmp);
		    tmp = 0;
		}
		if (!rn2(33)) tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
		break;
	    case AD_ELEC:
		if (nohit) break;
		
		if (cancelled) {
		    tmp = 0;
		    break;
		}
		if (vis) pline("%s gets zapped!", Monnam(mdef));
		if (!rn2(33)) tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
		if (resists_elec(mdef)) {
		    if (vis) pline_The("zap doesn't shock %s!", mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_ELEC, tmp);
		    tmp = 0;
		}
		/* only rings damage resistant players in destroy_item */
		if (!rn2(33)) tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
		break;
	    case AD_ACID:
		if (nohit) break;
		
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		if (resists_acid(mdef)) {
		    if (vis)
			pline("%s is covered in acid, but it seems harmless.",
			      Monnam(mdef));
		    tmp = 0;
		} else if (vis) {
		    pline("%s is covered in acid!", Monnam(mdef));
		    pline("It burns %s!", mon_nam(mdef));
		}
		if (!rn2(30)) erode_armor(mdef, TRUE);
		if (!rn2(6)) erode_obj(MON_WEP(mdef), TRUE, TRUE);
		break;
	    case AD_RUST:
		if (magr->mcan) break;
		if (pd == &mons[PM_IRON_GOLEM]) {
			if (vis) pline("%s falls to pieces!", Monnam(mdef));
			mondied(mdef);
			if (mdef->mhp > 0) return 0;
			else if (mdef->mtame && !vis)
			    pline("May %s rust in peace.", mon_nam(mdef));
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		hurtmarmor(mdef, AD_RUST);
		mdef->mstrategy &= ~STRAT_WAITFORU;
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		if (pd == &mons[PM_IRON_GOLEM]) tmp = 0;
		break;
	    case AD_LITE:
		if (is_vampire(mdef->data)) {
			tmp *= 2; /* vampires take more damage from sunlight --Amy */
			if (vis) pline("%s is irradiated!", Monnam(mdef));
		}
		break;

	    case AD_CORR:
		if (magr->mcan) break;
		hurtmarmor(mdef, AD_CORR);
		mdef->mstrategy &= ~STRAT_WAITFORU;
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		/*tmp = 0;*/
		break;
	    case AD_DCAY:
		if (magr->mcan) break;
		if (pd == &mons[PM_WOOD_GOLEM] ||
		    pd == &mons[PM_LEATHER_GOLEM]) {
			if (vis) pline("%s falls to pieces!", Monnam(mdef));
			mondied(mdef);
			if (mdef->mhp > 0) return 0;
			else if (mdef->mtame && !vis)
			    pline("May %s rot in peace.", mon_nam(mdef));
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		hurtmarmor(mdef, AD_DCAY);
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		if (pd == &mons[PM_WOOD_GOLEM] || pd == &mons[PM_LEATHER_GOLEM]) tmp = 0;
		break;
	    case AD_STON:
	    case AD_EDGE:
		if (magr->mcan) break;
		if (mattk->aatyp == AT_GAZE && mon_reflects(mdef, (char *)0)) {
		    tmp = 0;
		    (void) mon_reflects(mdef, "But it reflects from %s %s!");
		    if (poly_when_stoned(pa)) {
			mon_to_stone(magr);
			break;
		    }
		    if (!resists_ston(magr) && !rn2(4) ) {
			if (vis) pline("%s turns to stone!", Monnam(magr));
			monstone(magr);
			if (magr->mhp > 0) return 0;
			else if (magr->mtame && !vis)
			    You(brief_feeling, "peculiarly sad");
			return MM_AGR_DIED;
		    }
		}
 do_stone:
		/* may die from the acid if it eats a stone-curing corpse */
		if (munstone(mdef, FALSE)) goto post_stone;
		if (poly_when_stoned(pd)) {
			mon_to_stone(mdef);
			tmp = 0;
			break;
		}
		if (!resists_ston(mdef) && !rn2(4) ) {
			if (vis) pline("%s turns to stone!", Monnam(mdef));
			monstone(mdef);
 post_stone:		if (mdef->mhp > 0) return 0;
			else if (mdef->mtame && !vis)
			    You(brief_feeling, "peculiarly sad");
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
			tmp = (mattk->adtyp == AD_STON ? 0 : mattk->adtyp == AD_EDGE ? 0 : 1);
		}
		break;
	    case AD_BANI:
		if (mdef->mtame && !rn2(3)) mdef->willbebanished = TRUE;
		break;
	    case AD_TLPT:
	    case AD_NEXU:
	    case AD_ABDC:
		if (!cancelled && tmp < mdef->mhp && !tele_restrict(mdef)) {
		    char mdef_Monnam[BUFSZ];
		    /* save the name before monster teleports, otherwise
		       we'll get "it" in the suddenly disappears message */
		    if (vis) strcpy(mdef_Monnam, Monnam(mdef));
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    (void) rloc(mdef, FALSE);
		    if (vis && !canspotmon(mdef) && mdef != u.usteed )
			pline("%s suddenly disappears!", mdef_Monnam);
		}
		break;
	    case AD_SLEE:
		if (nohit) break;                
		
		if (cancelled) break;
		if (mattk->aatyp == AT_GAZE && mon_reflects(mdef, (char *)0)) {
		    tmp = 0;
		    (void) mon_reflects(mdef, "But it reflects from %s %s!");
		    if (sleep_monst(magr, rnd(10), -1))
			if (vis) pline("%s is put to sleep!", Monnam(magr));
		    break;
		}

		if (!cancelled && !mdef->msleeping &&
			sleep_monst(mdef, rnd(10), -1)) {
		    if (vis) {
			strcpy(buf, Monnam(mdef));
			pline("%s is put to sleep by %s.", buf, mon_nam(magr));
		    }
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    slept_monst(mdef);
		}
		break;
	    /* WAC DEATH (gaze) */
	    case AD_DETH:
		if (rn2(16)) {
		    /* No death, but still cause damage */
		    break;
		} 
		if (vis && mattk->aatyp == AT_GAZE) 
		    pline("%s gazes intently!", Monnam(magr));
		if (mattk->aatyp == AT_GAZE && mon_reflects(mdef, (char *)0)) {
		    /* WAC reflected gaze 
		     * Oooh boy...that was a bad move :B 
		     */
		    tmp = 0;
		    if (vis) {
			shieldeff(mdef->mx, mdef->my);
			(void) mon_reflects(mdef, "But it reflects from %s %s!");
		    }
		    if (resists_magm(magr)) {
			if (vis) pline("%s shudders momentarily...", Monnam(magr));
			break;
		    }
		    if (vis) pline("%s dies!", Monnam(magr));
		    mondied(magr);
		    if (magr->mhp > 0) return 0;  /* lifesaved */
		    else if (magr->mtame && !vis)
			You(brief_feeling, "peculiarly sad");
		    return MM_AGR_DIED;
		} else if (is_undead(mdef->data) || mdef->egotype_undead) {
		    /* Still does normal damage */
		    if (vis) pline("Something didn't work...");
		    break;
		} else if (resists_magm(mdef)) {
		    if (vis) pline("%s shudders momentarily...", Monnam(mdef));
		} else {
		    tmp = mdef->mhp;
		}
		break;
	    case AD_PLYS:
		if (nohit) break;                
		if(!cancelled && mdef->mcanmove && !(dmgtype(mdef->data, AD_PLYS))) {
		    if (vis) {
			strcpy(buf, Monnam(mdef));
			pline("%s is frozen by %s.", buf, mon_nam(magr));
		    }
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		break;
	    case AD_TCKL:
		if(!cancelled && mdef->mcanmove && !(dmgtype(mdef->data, AD_PLYS))) {
		    if (vis) {
			strcpy(buf, Monnam(magr));
			pline("%s mercilessly tickles %s.", buf, mon_nam(mdef));
		    }
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
  		}
		break;
	    case AD_SLOW:
	    case AD_WGHT:
	    case AD_INER:
		if (nohit) break;
		if(!cancelled && vis && mdef->mspeed != MSLOW) {
		    unsigned int oldspeed = mdef->mspeed;

		    mon_adjust_speed(mdef, -1, (struct obj *)0);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    if (mdef->mspeed != oldspeed && vis)
			pline("%s slows down.", Monnam(mdef));
		}
		break;
	    case AD_LAZY:
		if (nohit) break;
		if(!cancelled && vis && mdef->mspeed != MSLOW) {
		    unsigned int oldspeed = mdef->mspeed;

		    mon_adjust_speed(mdef, -1, (struct obj *)0);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    if (mdef->mspeed != oldspeed && vis)
			pline("%s slows down.", Monnam(mdef));
		}
		if(!cancelled && !rn2(3) && mdef->mcanmove && !(dmgtype(mdef->data, AD_PLYS))) {
		    if (vis) {
			strcpy(buf, Monnam(mdef));
			pline("%s is frozen by %s.", buf, mon_nam(magr));
		    }
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		break;
	    case AD_NUMB:
		if (nohit) break;
		if(!cancelled && !rn2(10) && vis && mdef->mspeed != MSLOW) {
		    unsigned int oldspeed = mdef->mspeed;

		    mon_adjust_speed(mdef, -1, (struct obj *)0);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    if (mdef->mspeed != oldspeed && vis)
			pline("%s is numbed.", Monnam(mdef));
		}
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		break;
	    case AD_DARK:
		do_clear_area(mdef->mx,mdef->my, 7, set_lit, (void *)((char *)0));
		if (vis) pline("A sinister darkness fills the area!");
		if (mdef->data->mlet == S_ANGEL) tmp *= 2;
		break;

	    case AD_THIR:
	    case AD_NTHR:
		if (magr->mhp > 0) {
		magr->mhp += tmp;
		if (magr->mhp > magr->mhpmax) magr->mhp = magr->mhpmax;
		if (vis) pline("%s feeds on the lifeblood!", Monnam(magr) );
		}

		break;

	    case AD_RAGN:

		ragnarok(FALSE);
		if (evilfriday && magr->m_lev > 1) evilragnarok(FALSE,magr->m_lev);
		break;

	    case AD_AGGR:
		aggravate();
		if (!rn2(20)) {
			u.aggravation = 1;
			reset_rndmonst(NON_PM);
			(void) makemon((struct permonst *)0, magr->mx, magr->my, MM_ANGRY|MM_ADJACENTOK|MM_FRENZIED);
			u.aggravation = 0;
		}

		break;

	    case AD_CONT:

		if (!rn2(30)) {
			mdef->isegotype = 1;
			mdef->egotype_contaminator = 1;
		}
		if (!rn2(100)) {
			mdef->isegotype = 1;
			mdef->egotype_weeper = 1;
		}
		if (!rn2(250)) {
			mdef->isegotype = 1;
			mdef->egotype_radiator = 1;
		}
		if (!rn2(250)) {
			mdef->isegotype = 1;
			mdef->egotype_reactor = 1;
		}

		break;

	    case AD_FRZE:
		if (!resists_cold(mdef) && resists_fire(mdef)) {
			tmp *= 2;
			if (vis) pline("%s is freezing!", Monnam(mdef));
		}
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;

		break;
	    case AD_ICEB:
		if (!resists_cold(mdef)) {
			tmp *= 2;
			if (vis) pline("%s is hit with ice blocks!", Monnam(mdef));
		}

		break;

	    case AD_MALK:
		if (!resists_elec(mdef)) {
			tmp *= 2;
			if (vis) pline("%s is shocked!", Monnam(mdef));
		}

		break;

	    case AD_UVUU:
		if (has_head(mdef->data)) {
			tmp *= 2;
			if (!rn2(1000)) {
				tmp *= 100;
				if (vis) pline("%s's %s is torn apart!", Monnam(mdef), mbodypart(mdef, HEAD));
			} else if (vis) pline("%s's %s is spiked!", Monnam(mdef), mbodypart(mdef, HEAD));
		}
		break;

	    case AD_GRAV:
		if (!is_flyer(mdef->data)) {
			tmp *= 2;
			if (vis) pline("%s is slammed into the ground!", Monnam(mdef));
		}
		break;

	    case AD_CHKH:
		if (magr->m_lev > mdef->m_lev) tmp += (magr->m_lev - mdef->m_lev);
		break;

	    case AD_CHRN:
		if ((tmp > 0) && (mdef->mhpmax > 1)) {
			mdef->mhpmax--;
			if (vis) pline("%s feels bad!", Monnam(mdef));
		}
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		break;

	    case AD_HODS:
		tmp += mdef->m_lev;
		break;

	    case AD_DIMN:
		tmp += magr->m_lev;
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		break;

	    case AD_BURN:
		if (resists_cold(mdef) && !resists_fire(mdef)) {
			tmp *= 2;
			if (vis) pline("%s is burning!", Monnam(mdef));
		}
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;

		break;

	    case AD_PLAS:
		if (!resists_fire(mdef)) {
			tmp *= 2;
			if (vis) pline("%s is enveloped by searing plasma radiation!", Monnam(mdef));
		}

		break;

	    case AD_SLUD:
		if (!resists_acid(mdef)) {
			tmp *= 2;
			if (vis) pline("%s is covered with sludge!", Monnam(mdef));
		}

		break;

	    case AD_LAVA:
		if (resists_cold(mdef) && !resists_fire(mdef)) {
			tmp *= 4;
			if (vis) pline("%s is scorched!", Monnam(mdef));
		} else if (!resists_fire(mdef)) {
			tmp *= 2;
			if (vis) pline("%s is severely burned!", Monnam(mdef));
		}

		break;

	    case AD_FAKE:
		pline("%s", fauxmessage());
		if (!rn2(3)) pline("%s", fauxmessage());

		break;

	    case AD_WEBS:
		(void) maketrap(mdef->mx, mdef->my, WEB, 0);
		if (!rn2(issoviet ? 2 : 8)) makerandomtrap();

		break;

	    case AD_TRAP:
		if (t_at(mdef->mx, mdef->my) == 0) (void) maketrap(mdef->mx, mdef->my, randomtrap(), 0);
		else makerandomtrap();

		break;

	    case AD_CNCL:
		if (rnd(100) > mdef->data->mr) {
			mdef->mcan = 1;
			if (vis) pline("%s is covered in sparkling lights!", Monnam(mdef));
		}

		break;

	    case AD_FEAR:
		if (rnd(100) > mdef->data->mr) {
		     monflee(mdef, rnd(1 + tmp), FALSE, TRUE);
			if (vis) pline("%s screams in fear!",Monnam(mdef));
		}
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;

		break;

	    case AD_SANI:
		if (!rn2(10)) {
			mdef->mconf = 1;
			if (vis) {
			switch (rnd(4)) {

				case 1:
					pline("%s sees %s chow dead bodies.", Monnam(mdef), mon_nam(magr)); break;
				case 2:
					pline("%s shudders at %s's terrifying %s.", Monnam(mdef), mon_nam(magr), makeplural(mbodypart(magr, EYE)) ); break;
				case 3:
					pline("%s feels sick at entrails caught in %s's tentacles.", Monnam(mdef), mon_nam(magr)); break;
				case 4:
					pline("%s sees maggots breed in the rent %s of %s.", Monnam(mdef), mbodypart(magr, STOMACH), mon_nam(magr)); break;

			}
			}

		}

		break;

	    case AD_INSA:
		if (rnd(100) > mdef->data->mr) {
		     monflee(mdef, rnd(1 + tmp), FALSE, TRUE);
			if (vis) pline("%s screams in fear!",Monnam(mdef));
		}
		if (!magr->mcan && !mdef->mconf && !magr->mspec_used) {
		    if (vis) pline("%s looks confused.", Monnam(mdef));
		    mdef->mconf = 1;
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		if (!magr->mcan && canseemon(mdef))
		    pline("%s %s for a moment.", Monnam(mdef), makeplural(stagger(mdef->data, "stagger")));
		mdef->mstun = 1;

		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;

		break;

	    case AD_DREA:
		if (!mdef->mcanmove) {
			tmp *= 4;
			if (vis) pline("%s's dream is eaten!",Monnam(mdef));
		}

		break;

	    case AD_CONF:
	    case AD_SPC2:
		if (nohit) break;
		/* Since confusing another monster doesn't have a real time
		 * limit, setting spec_used would not really be right (though
		 * we still should check for it).
		 */
		if (!magr->mcan && !mdef->mconf && !magr->mspec_used) {
		    if (vis) pline("%s looks confused.", Monnam(mdef));
		    mdef->mconf = 1;
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		break;

	    case AD_FAMN:
		if (mdef->mtame) {
			makedoghungry(mdef, tmp * rnd(50));
			if (vis) pline("%s suddenly looks hungry.", Monnam(mdef));
		}
		break;

	    case AD_WRAT:
	    case AD_MANA:
	    case AD_TECH:
	    case AD_MEMO:
	    case AD_TRAI:
	    	    mon_drain_en(mdef, ((mdef->m_lev > 0) ? (rnd(mdef->m_lev)) : 0) + 1 + tmp);
		break;

	    case AD_DREN:
		if (nohit) break;
	    	if (resists_magm(mdef)) {
		    if (vis) {
			shieldeff(mdef->mx,mdef->my);
			pline("%s is unaffected.", Monnam(mdef));
		    }
	    	} else {
	    	    mon_drain_en(mdef, 
				((mdef->m_lev > 0) ? (rnd(mdef->m_lev)) : 0) + 1);
	    	}	    
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		break;
	    case AD_BLND:
		if (nohit) break;                
	       
		if (can_blnd(magr, mdef, mattk->aatyp, (struct obj*)0)) {
		    register unsigned rnd_tmp;

		    if (vis && mdef->mcansee)
			pline("%s is blinded.", Monnam(mdef));
		    rnd_tmp = d((int)mattk->damn, (int)mattk->damd);
		    if ((rnd_tmp += mdef->mblinded) > 127) rnd_tmp = 127;
		    mdef->mblinded = rnd_tmp;
		    mdef->mcansee = 0;
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		/*tmp = 0;*/
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		break;
	    case AD_HALU:
	    case AD_DEPR:
		if (!magr->mcan && haseyes(pd) && mdef->mcansee) {
		    if (vis) pline("%s looks %sconfused.",
				    Monnam(mdef), mdef->mconf ? "more " : "");
		    mdef->mconf = 1;
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		/*tmp = 0;*/
		if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1;
		break;
	    case AD_CURS:
	    case AD_ICUR:
	    case AD_NACU:
		if (nohit) break;
		
		if (!night() && (pa == &mons[PM_GREMLIN])) break;
		if (!magr->mcan && !rn2(10) && (rnd(100) > mdef->data->mr) ) {
		    mdef->mcan = 1;	/* cancelled regardless of lifesave */
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    if (is_were(pd) && pd->mlet != S_HUMAN)
			were_change(mdef);
		    if (pd == &mons[PM_CLAY_GOLEM]) {
			    if (vis) {
				pline("Some writing vanishes from %s head!",
				    s_suffix(mon_nam(mdef)));
				pline("%s is destroyed!", Monnam(mdef));
			    }
			    mondied(mdef);
			    if (mdef->mhp > 0) return 0;
			    else if (mdef->mtame && !vis)
				You(brief_feeling, "strangely sad");
			    return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		    }
		    if (flags.soundok) {
			    if (!vis) You_hear("laughter.");
			    else pline("%s chuckles.", Monnam(magr));
				if (PlayerHearsSoundEffects) pline(issoviet ? "Tip bloka l'da smeyetsya tozhe." : "Hoehoehoehoe!");
		    }
		}
		break;
	    case AD_SGLD:
		tmp = 0;
#ifndef GOLDOBJ
		if (magr->mcan || !mdef->mgold) break;
		/* technically incorrect; no check for stealing gold from
		 * between mdef's feet...
		 */
		magr->mgold += mdef->mgold;
		mdef->mgold = 0;
#else
                if (magr->mcan) break;
		/* technically incorrect; no check for stealing gold from
		 * between mdef's feet...
		 */
                {
		    struct obj *gold = findgold(mdef->minvent);
		    if (!gold) break;
                    obj_extract_self(gold);
		    add_to_minv(magr, gold);
                }
#endif
		mdef->mstrategy &= ~STRAT_WAITFORU;
		if (vis) {
		    strcpy(buf, Monnam(magr));
		    pline("%s steals some gold from %s.", buf, mon_nam(mdef));
		}
		if (!tele_restrict(magr)) {
		    (void) rloc(magr, FALSE);
		    if (vis && !canspotmon(magr))
			pline("%s suddenly disappears!", buf);
		}
		break;

	    case AD_PAIN:
		if (mdef->mhp > 9) tmp += (mdef->mhp / 10);
		if (vis) pline("%s shrieks in pain!", Monnam(mdef));
		break;

	    case AD_DRLI:
	    case AD_TIME:
	    case AD_DFOO:
	    case AD_WEEP:
	    case AD_VAMP:
		if (nohit) break;                

		if (!cancelled && magr->mtame && !magr->isminion &&
			is_vampire(pa) && mattk->aatyp == AT_BITE &&
			has_blood(pd))
		    EDOG(magr)->hungrytime += ((int)((mdef->data)->cnutrit / 20) + 1);
		
		if (!cancelled && rn2(2) && !resists_drli(mdef)) {
			tmp = d(2,6);
			if (vis)
			    pline("%s suddenly seems weaker!", Monnam(mdef));
			mdef->mhpmax -= tmp;
			if (mdef->m_lev == 0)
				tmp = mdef->mhp;
			else mdef->m_lev--;
			/* Automatic kill if drained past level 0 */
		}
		break;
	    case AD_SSEX:
	    case AD_SITM:	/* for now these are the same */
	    case AD_SEDU:
	    case AD_STTP:
		if (magr->mcan) break;
		/* find an object to steal, non-cursed if magr is tame */
		for (obj = mdef->minvent; obj; obj = obj->nobj)
		    if (!magr->mtame || !obj->cursed)
			break;

		if (obj) {
			char onambuf[BUFSZ], mdefnambuf[BUFSZ];

			/* make a special x_monnam() call that never omits
			   the saddle, and save it for later messages */
			strcpy(mdefnambuf, x_monnam(mdef, ARTICLE_THE, (char *)0, 0, FALSE));

			otmp = obj;
			if (u.usteed == mdef &&
					otmp == which_armor(mdef, W_SADDLE)) {

	/* I took the liberty of making saddles less likely to be stolen, but for a long time that code was only in steal.c
	 * and therefore never actually did anything. Now, your steed should no longer be super vulnerable
	 * to those motherfucker item-stealers! --Amy */
				if (rn2(5) && !issoviet) break;

				/* "You can no longer ride <steed>." */
				dismount_steed(DISMOUNT_POLY);
			}
			obj_extract_self(otmp);
			if (otmp->owornmask) {
				mdef->misc_worn_check &= ~otmp->owornmask;
				if (otmp->owornmask & W_WEP)
				    setmnotwielded(mdef,otmp);
				otmp->owornmask = 0L;
				update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
			}
			/* add_to_minv() might free otmp [if it merges] */
			if (vis)
				strcpy(onambuf, doname(otmp));
			(void) add_to_minv(magr, otmp);
			if (vis) {
				strcpy(buf, Monnam(magr));
				pline("%s steals %s from %s!", buf,
				    onambuf, mdefnambuf);
			}
			possibly_unwield(mdef, FALSE);
			mdef->mstrategy &= ~STRAT_WAITFORU;
			mselftouch(mdef, (const char *)0, FALSE);
			if (mdef->mhp <= 0)
				return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
			if (magr->data->mlet == S_NYMPH &&
			    !tele_restrict(magr) && !rn2(5) ) {
			    (void) rloc(magr, FALSE);
			    if (vis && !canspotmon(magr))
				pline("%s suddenly disappears!", buf);
			}
		}
		tmp = 0;
		break;
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
	    case AD_POIS:
	    case AD_STAT:
	    case AD_WISD:
	    case AD_DRCH:
		if (nohit) break;
		
		if (!cancelled && !rn2(8)) {
		    if (vis)
			pline("%s %s was poisoned!", s_suffix(Monnam(magr)),
			      mpoisons_subj(magr, mattk));
		    if (resists_poison(mdef)) {
			if (vis)
			    pline_The("poison doesn't seem to affect %s.",
				mon_nam(mdef));
		    } else {
			if (rn2(100)) tmp += rn1(10,6);
			else {
			    if (vis) pline_The("poison was deadly...");
			    tmp = mdef->mhp;
			}
		    }
		}
		break;
	    case AD_VENO:
		if (nohit) break;
		
		if (!cancelled && !rn2(3)) {
		    if (resists_poison(mdef)) {
			if (vis)
			    pline_The("poison doesn't seem to affect %s.",
				mon_nam(mdef));
		    } else {
			if (vis) pline("%s is badly poisoned!", Monnam(mdef));
			if (rn2(10)) tmp += rn1(20,12);
			else {
			    if (vis) pline_The("poison was deadly...");
			    tmp = mdef->mhp;
			}
		    }
		}
		break;

	    case AD_DRIN:
		if (notonhead || !has_head(pd)) {
		    if (vis) pline("%s doesn't seem harmed.", Monnam(mdef));
		    /* Not clear what to do for green slimes */
		    tmp = 0;
		    break;
		}
		if ((mdef->misc_worn_check & W_ARMH) && rn2(8)) {
		    if (vis) {
			strcpy(buf, s_suffix(Monnam(mdef)));
			pline("%s helmet blocks %s attack to %s head.",
				buf, s_suffix(mon_nam(magr)),
				mhis(mdef));
		    }
		    break;
		}
		if (vis) pline("%s brain is eaten!", s_suffix(Monnam(mdef)));
		if (mindless(pd) || mdef->egotype_undead ) {
		    if (vis) pline("%s doesn't notice.", Monnam(mdef));
		    break;
		}
		tmp += rnd(10); /* fakery, since monsters lack INT scores */
		if (magr->mtame && !magr->isminion) {
		    EDOG(magr)->hungrytime += rnd(60);
		    magr->mconf = 0;
		}
		if (tmp >= mdef->mhp && vis)
		    pline("%s last thought fades away...",
			          s_suffix(Monnam(mdef)));
		break;
	    case AD_SLIM: /* no easy sliming Death or Famine --Amy */
	    case AD_LITT:
		if (cancelled || (rn2(100) < mdef->data->mr) ) break;   /* physical damage only */
		if (!rn2(400) && !flaming(mdef->data) &&
				!slime_on_touch(mdef->data) ) {
		    if (newcham(mdef, &mons[PM_GREEN_SLIME], FALSE, vis)) {
			mdef->oldmonnm = PM_GREEN_SLIME;
			(void) stop_timer(UNPOLY_MON, (void *) mdef);
		    }
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    tmp = 0;
		}
		break;
	    case AD_STCK:
		if (cancelled) tmp = 0;
		break;
	    case AD_WRAP: /* monsters cannot grab one another, it's too hard */
		if (magr->mcan) tmp = 0;
		break;
	    case AD_ENCH:
		/* There's no msomearmor() function, so just do damage */
	     /* if (cancelled) break; */
		break;
	    case AD_POLY:
		if (!magr->mcan && tmp < mdef->mhp) {
		    if (resists_magm(mdef) || (rn2(100) < mdef->data->mr) ) { /* no easy taming Death or Famine! --Amy */
			/* magic resistance protects from polymorph traps, so
			 * make it guard against involuntary polymorph attacks
			 * too... */
			if (vis) shieldeff(mdef->mx, mdef->my);
			break;
		    }
#if 0
		    if (!rn2(25) || !mon_poly(mdef)) {
			if (vis)
			    pline("%s shudders!", Monnam(mdef));
			/* no corpse after system shock */
			tmp = rnd(30);
		    } else 
#endif
		    (void) mon_poly(mdef, FALSE,
			    "%s undergoes a freakish metamorphosis!");
		}
		break;

	    case AD_CHAO:
		if (!magr->mcan && tmp < mdef->mhp) {
		    if (resists_magm(mdef) || (rn2(100) < mdef->data->mr) ) { /* no easy taming Death or Famine! --Amy */
			/* magic resistance protects from polymorph traps, so
			 * make it guard against involuntary polymorph attacks
			 * too... */
			if (vis) shieldeff(mdef->mx, mdef->my);
			break;
		    }
		    (void) mon_poly(mdef, FALSE,
			    "%s undergoes a freakish metamorphosis!");
		}
		if ((tmp > 0) && (mdef && mdef->mhpmax > 1)) {
			mdef->mhpmax--;
			if (vis) pline("%s feels bad!", Monnam(mdef));
		}

		break;

	    case AD_CALM:	/* KMH -- koala attack */

		{
		int untamingchance = 10;

		if (!(PlayerCannotUseSkills)) {
			switch (P_SKILL(P_PETKEEPING)) {
				default: untamingchance = 10; break;
				case P_BASIC: untamingchance = 9; break;
				case P_SKILLED: untamingchance = 8; break;
				case P_EXPERT: untamingchance = 7; break;
				case P_MASTER: untamingchance = 6; break;
				case P_GRAND_MASTER: untamingchance = 5; break;
				case P_SUPREME_MASTER: untamingchance = 4; break;
			}
		}

		/* Certain monsters aren't even made peaceful. */
		if (!mdef->iswiz && mdef->data != &mons[PM_MEDUSA] &&
			!(mdef->data->mflags3 & M3_COVETOUS) &&
			!(mdef->data->geno & G_UNIQ) &&
			((magr->mtame && !rn2(10)) || (mdef->mtame && (untamingchance > rnd(10)) && !((rnd(30 - ACURR(A_CHA))) < 4)) )) {
		    if (vis) pline("%s looks calmer.", Monnam(mdef));
		    if (mdef == u.usteed && !mayfalloffsteed())
			dismount_steed(DISMOUNT_THROWN);
		    if (!mdef->mfrenzied) mdef->mpeaceful = 1;
		    mdef->mtame = 0;
		    tmp = 0;
		}

		}

		break;
	    case AD_FREN:

		{
		int untamingchance = 10;

		if (!(PlayerCannotUseSkills)) {
			switch (P_SKILL(P_PETKEEPING)) {
				default: untamingchance = 10; break;
				case P_BASIC: untamingchance = 9; break;
				case P_SKILLED: untamingchance = 8; break;
				case P_EXPERT: untamingchance = 7; break;
				case P_MASTER: untamingchance = 6; break;
				case P_GRAND_MASTER: untamingchance = 5; break;
				case P_SUPREME_MASTER: untamingchance = 4; break;
			}
		}

		if (!mdef->mfrenzied && (!mdef->mtame || (mdef->mtame <= rnd(21) && (untamingchance > rnd(10)) && !((rnd(30 - ACURR(A_CHA))) < 4) ) ) ) {
			mdef->mpeaceful = mdef->mtame = 0;
			mdef->mfrenzied = 1;
		    if (vis) pline("%s enters a state of frenzy!", Monnam(mdef));
		}

		}

		break;
	    default:	/*tmp = 0;*/ 
			if (mattk->aatyp == AT_EXPL && tmp > 1) tmp = 1; /* fail safe */
			break; /* necessary change to make pets more viable --Amy */
	}
	if(!tmp) return(MM_MISS);

	/* STEPHEN WHITE'S NEW CODE */
	if (objenchant < canhitmon && vis && nohit) {
			strcpy(buf, Monnam(magr));
			pline("%s doesn't seem to harm %s.", buf,
								mon_nam(mdef));
		return(MM_HIT);
	}
	/* WAC -- Caveman Primal Roar ability */
	if (magr->mtame != 0 && tech_inuse(T_PRIMAL_ROAR)) {
		tmp *= 2; /* Double Damage! */
	}
	if((mdef->mhp -= tmp) < 1) {
	    if (m_at(mdef->mx, mdef->my) == magr) {  /* see gulpmm() */
		remove_monster(mdef->mx, mdef->my);
		mdef->mhp = 1;	/* otherwise place_monster will complain */
		place_monster(mdef, mdef->mx, mdef->my);
		mdef->mhp = 0;
	    }
	    /* get experience from spell creatures */
	    if (magr->uexp) mon_xkilled(mdef, "", (int)mattk->adtyp);
	    else monkilled(mdef, "", (int)mattk->adtyp);

	    if (mdef->mhp > 0) return 0; /* mdef lifesaved */

	    if (magr->mhp > 0 && magr->mtame) use_skill(P_PETKEEPING,1);

	    if (mattk->adtyp == AD_DGST) { 
		/* various checks similar to dog_eat and meatobj.
		 * after monkilled() to provide better message ordering */
		if (mdef->cham != CHAM_ORDINARY) {
		    (void) newcham(magr, (struct permonst *)0, FALSE, TRUE);
		} else if (mdef->data == &mons[PM_GREEN_SLIME]) {
		    (void) newcham(magr, &mons[PM_GREEN_SLIME], FALSE, TRUE);
		} else if (mdef->data == &mons[PM_WRAITH]) {
		    (void) grow_up(magr, (struct monst *)0);
		    /* don't grow up twice */
		    return (MM_DEF_DIED | (magr->mhp > 0 ? 0 : MM_AGR_DIED));
		} else if (mdef->data == &mons[PM_NURSE]) {
		    magr->mhp = magr->mhpmax;
		}
	    }

	    return (MM_DEF_DIED |
		    ((magr->mhp > 0 && grow_up(magr,mdef)) ? 0 : MM_AGR_DIED));
	}
	return(MM_HIT);
}

#endif /* OVLB */


#ifdef OVL0

int
noattacks(ptr)			/* returns 1 if monster doesn't attack */
	struct	permonst *ptr;
{
	int i;

	for(i = 0; i < NATTK; i++)
		if(ptr->mattk[i].aatyp) return(0);

	return(1);
}

/* `mon' is hit by a sleep attack; return 1 if it's affected, 0 otherwise */
int
sleep_monst(mon, amt, how)
struct monst *mon;
int amt, how;
{
	if (resists_sleep(mon) ||
		(how >= 0 && resist(mon, (char)how, 0, NOTELL))) {
	    shieldeff(mon->mx, mon->my);
	} else if (mon->mcanmove) {
	    amt += (int) mon->mfrozen;
	    if (amt > 0) {	/* sleep for N turns */
		mon->mcanmove = 0;
		mon->mfrozen = min(amt, 127);
		mon->masleep = 1;
	    } else {		/* sleep until awakened */
		mon->msleeping = 1;
	    }
	    return 1;
	}
	return 0;
}

/* sleeping grabber releases, engulfer doesn't; don't use for paralysis! */
void
slept_monst(mon)
struct monst *mon;
{
	if ((mon->msleeping || !mon->mcanmove) && mon == u.ustuck &&
		!sticks(youmonst.data) && !u.uswallow) {
	    pline("%s grip relaxes.", s_suffix(Monnam(mon)));
	    unstuck(mon);
	}
}

#endif /* OVL0 */
#ifdef OVLB

STATIC_OVL void
mrustm(magr, mdef, obj)
register struct monst *magr, *mdef;
register struct obj *obj;
{
	boolean is_acid;

	if (!magr || !mdef || !obj) return; /* just in case */

	/* It is just teh uber cheat0r that non-passive rusting attacks still cause the attacking monster's shit to rust. */
	if (attackdamagetype(mdef->data, AT_NONE, AD_CORR)) {
	    is_acid = TRUE;
	} else if (attackdamagetype(mdef->data, AT_NONE, AD_RUST)) {
	    is_acid = FALSE;
	} else if (attackdamagetype(mdef->data, AT_RATH, AD_CORR)) {
	    is_acid = TRUE;
	} else if (attackdamagetype(mdef->data, AT_RATH, AD_RUST)) {
	    is_acid = FALSE;

	/* In Soviet Russia, the Amy is considered the antichrist and everything she does must be bad. She can go ahead and
	 * do obvious bug fixes that every sane person would immediately recognize as such, but the type of ice block goes
	 * ahead and says 'she made this change, therefore it must be bad REVERTREVERTREVERTREVERTREVERTREVERTREVERT'. */
	} else if (issoviet) {
		if (dmgtype(mdef->data, AD_CORR))
		    is_acid = TRUE;
		else if (dmgtype(mdef->data, AD_RUST))
		    is_acid = FALSE;
		else return;
	} else
	    return;

	if (!mdef->mcan &&
	    (is_acid ? is_corrodeable(obj) : is_rustprone(obj)) && !stack_too_big(obj) &&
	    (is_acid ? obj->oeroded2 : obj->oeroded) < MAX_ERODE) {
		if (obj->greased || (obj->oartifact && rn2(4)) || obj->oerodeproof || (obj->blessed && rn2(3))) {
		    if (cansee(mdef->mx, mdef->my) && flags.verbose)
			pline("%s weapon is not affected.",
			                 s_suffix(Monnam(magr)));
		    if (obj->greased && !rn2(2)) obj->greased -= 1;
		} else {
		    if (cansee(mdef->mx, mdef->my)) {
			pline("%s %s%s!", s_suffix(Monnam(magr)),
			    aobjnam(obj, (is_acid ? "corrode" : "rust")),
			    (is_acid ? obj->oeroded2 : obj->oeroded)
				? " further" : "");
		    }
		    if (is_acid) obj->oeroded2++;
		    else obj->oeroded++;
		}
	}
}

STATIC_OVL void
mswingsm(magr, mdef, otemp)
register struct monst *magr, *mdef;
register struct obj *otemp;
{
	char buf[BUFSZ];
	if (!flags.verbose || Blind || !mon_visible(magr)) return;
	strcpy(buf, mon_nam(mdef));
	pline("%s %s %s %s at %s.", Monnam(magr),
	      (objects[otemp->otyp].oc_dir & PIERCE) ? "thrusts" : "swings",
	      mhis(magr), singular(otemp, xname), buf);
}

/*
 * Passive responses by defenders.  Does not replicate responses already
 * handled above.  Returns same values as mattackm.
 */
STATIC_OVL int
passivemm(magr,mdef,mhit,mdead,attnumber)
register struct monst *magr, *mdef;
boolean mhit;
int mdead;
int attnumber;
{
	register struct permonst *mddat = mdef->data;
	register struct permonst *madat = magr->data;
	char buf[BUFSZ];
	int i, tmp;

	int atttypB;

	if (mdef->mtame && !monnear(magr, mdef->mx, mdef->my)) return 0;

	for(i = 0; ; i++) {
	    if(i >= NATTK) return (mdead | mhit); /* no passive attacks */
	    if((i == attnumber) && mddat->mattk[i].aatyp == AT_NONE || mddat->mattk[i].aatyp == AT_RATH) break;
	}

	if (mddat->mattk[i].damn)
	    tmp = d((int)mddat->mattk[i].damn,
				    (int)mddat->mattk[i].damd);
	else if(mddat->mattk[i].damd)
	    tmp = d((int)mddat->mlevel+1, (int)mddat->mattk[i].damd);
	else
	    tmp = 0;

	atttypB = mddat->mattk[i].adtyp;

	if (magr->mtame) {

		if (atttypB == AD_RBRE) {
			while (atttypB == AD_ENDS || atttypB == AD_RBRE || atttypB == AD_WERE) {
				atttypB = randattack();
			}
		}

		if (atttypB == AD_DAMA) {
			atttypB = randomdamageattack();
		}

		if (atttypB == AD_THIE) {
			atttypB = randomthievingattack();
		}

		if (atttypB == AD_RNG) {
			while (atttypB == AD_ENDS || atttypB == AD_RNG || atttypB == AD_WERE) {
				atttypB = rn2(AD_ENDS); }
		}

		if (atttypB == AD_PART) atttypB = u.adpartattack;

		if (atttypB == AD_MIDI) {
			atttypB = mdef->m_id;
			if (atttypB < 0) atttypB *= -1;
			while (atttypB >= AD_ENDS) atttypB -= AD_ENDS;
			if (!(atttypB >= AD_PHYS && atttypB < AD_ENDS)) atttypB = AD_PHYS; /* fail safe --Amy */
			if (atttypB == AD_WERE) atttypB = AD_PHYS;
		}
	}

	/* These affect the enemy even if defender killed */
	switch(atttypB) {
	    case AD_ACID:
		if (mhit && !rn2(2)) {
		    strcpy(buf, Monnam(magr));
		    if(canseemon(magr))
			pline("%s is splashed by %s acid!",
			      buf, s_suffix(mon_nam(mdef)));
		    if (resists_acid(magr)) {
			if(canseemon(magr))
			    pline("%s is not affected.", Monnam(magr));
			tmp = 0;
		    }
		} else tmp = 0;
		break;
		case AD_MAGM:
	    /* wrath of gods for attacking Oracle */
	    if(resists_magm(magr)) {
		if(canseemon(magr)) {
		shieldeff(magr->mx, magr->my);
		pline("A hail of magic missiles narrowly misses %s!",mon_nam(magr));
		}
	    } else {
		if(canseemon(magr))
			pline(magr->data == &mons[PM_WOODCHUCK] ? "ZOT!" : 
			"%s is hit by magic missiles appearing from thin air!",Monnam(magr));
		break;
	    }
	    break;
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
	    case AD_NGEN:
		if (mhit && !mdef->mcan && otmp) {
				drain_item(otmp);
		    /* No message */
		}
		break;
	    default:
		break;
	}
	if (mdead || mdef->mcan) return (mdead|mhit);

	/* These affect the enemy only if defender is still alive */
	if (rn2(3)) switch(atttypB) {
	    case AD_PLYS: /* Floating eye */

		if (dmgtype(magr->data, AD_PLYS)) return 1;

		if (tmp > 127) tmp = 127;
		if (mddat == &mons[PM_FLOATING_EYE]) {
		    /*if (!rn2(4)) tmp = 127;*/
		    if (magr->mcansee && haseyes(madat) && mdef->mcansee && !mdef->minvisreal &&
			(perceives(madat) || !mdef->minvis)) {
			sprintf(buf, "%s gaze is reflected by %%s %%s.",
				s_suffix(mon_nam(mdef)));
			if (mon_reflects(magr,
					 canseemon(magr) ? buf : (char *)0))
				return(mdead|mhit);
			strcpy(buf, Monnam(magr));
			if(canseemon(magr))
			    pline("%s is frozen by %s gaze!",
				  buf, s_suffix(mon_nam(mdef)));
			magr->mcanmove = 0;
			magr->mfrozen = tmp;
			return (mdead|mhit);
		    }
		} else { /* gelatinous cube */
		    strcpy(buf, Monnam(magr));
		    if(canseemon(magr))
			pline("%s is frozen by %s.", buf, mon_nam(mdef));
		    magr->mcanmove = 0;
		    magr->mfrozen = tmp;
		    return (mdead|mhit);
		}
		return 1;
	    case AD_COLD:
		if (resists_cold(magr)) {
		    if (canseemon(magr)) {
			pline("%s is mildly chilly.", Monnam(magr));
			golemeffects(magr, AD_COLD, tmp);
		    }
		    tmp = 0;
		    break;
		}
		if(canseemon(magr))
		    pline("%s is suddenly very cold!", Monnam(magr));
		mdef->mhp += tmp / 2;
		if (mdef->mhpmax < mdef->mhp) mdef->mhpmax = mdef->mhp;
		if (mdef->mhpmax > ((int) (mdef->m_lev+1) * 8) && !rn2(50) ) /* split much less often --Amy */
		    (void)split_mon(mdef, magr);
		break;
	    case AD_STUN:
		tmp = 0; /* fall through */
	    case AD_FUMB:
	    case AD_TREM:
	    case AD_SOUN:
		if (!magr->mstun) {
		    magr->mstun = 1;
		    if (canseemon(magr))
			pline("%s %s...", Monnam(magr),
			      makeplural(stagger(magr->data, "stagger")));
		}
		break;
	    case AD_FIRE:
		if (resists_fire(magr)) {
		    if (canseemon(magr)) {
			pline("%s is mildly warmed.", Monnam(magr));
			golemeffects(magr, AD_FIRE, tmp);
		    }
		    tmp = 0;
		    break;
		}
		if(canseemon(magr))
		    pline("%s is suddenly very hot!", Monnam(magr));
		break;
	    case AD_ELEC:
		if (resists_elec(magr)) {
		    if (canseemon(magr)) {
			pline("%s is mildly tingled.", Monnam(magr));
			golemeffects(magr, AD_ELEC, tmp);
		    }
		    tmp = 0;
		    break;
		}
		if(canseemon(magr))
		    pline("%s is jolted with electricity!", Monnam(magr));
		break;

	    case AD_LITE:
		if (is_vampire(magr->data)) {
			tmp *= 2; /* vampires take more damage from sunlight --Amy */
			if (canseemon(magr)) pline("%s is irradiated!", Monnam(magr));
		}
		break;
	    case AD_BANI:
		if (magr->mtame && !rn2(3)) magr->willbebanished = TRUE;
		break;
	    case AD_TLPT:
	    case AD_NEXU:
	    case AD_ABDC:
		if (!tele_restrict(magr)) (void) rloc(magr, FALSE);
		break;
	    case AD_SLEE:
		if (!magr->msleeping && sleep_monst(magr, rnd(10), -1)) {
		    if (canseemon(magr)) {
			pline("%s is put to sleep.", Monnam(magr));
		    }
		    magr->mstrategy &= ~STRAT_WAITFORU;
		    slept_monst(magr);
		}
		break;

	    case AD_SLOW:
	    case AD_WGHT:
	    case AD_INER:
		if(magr->mspeed != MSLOW) {
		    unsigned int oldspeed = magr->mspeed;

		    mon_adjust_speed(magr, -1, (struct obj *)0);
		    magr->mstrategy &= ~STRAT_WAITFORU;
		    if (magr->mspeed != oldspeed && canseemon(magr))
			pline("%s slows down.", Monnam(magr));
		}
		break;

	    case AD_LAZY:
		if(magr->mspeed != MSLOW) {
		    unsigned int oldspeed = magr->mspeed;

		    mon_adjust_speed(magr, -1, (struct obj *)0);
		    magr->mstrategy &= ~STRAT_WAITFORU;
		    if (magr->mspeed != oldspeed && canseemon(magr))
			pline("%s slows down.", Monnam(magr));
		}
		if(!rn2(3) && magr->mcanmove && !(dmgtype(magr->data, AD_PLYS))) {
		    if (canseemon(magr)) {
			pline("%s is paralyzed.", Monnam(magr));
		    }
		    magr->mcanmove = 0;
		    magr->mfrozen = rnd(10);
		    magr->mstrategy &= ~STRAT_WAITFORU;
		}
		break;

	    case AD_NUMB:
		if(!rn2(10) && magr->mspeed != MSLOW) {
		    unsigned int oldspeed = magr->mspeed;

		    mon_adjust_speed(magr, -1, (struct obj *)0);
		    magr->mstrategy &= ~STRAT_WAITFORU;
		    if (magr->mspeed != oldspeed && canseemon(magr))
			pline("%s is numbed.", Monnam(magr));
		}
		break;

	    case AD_DARK:
		do_clear_area(magr->mx,magr->my, 7, set_lit, (void *)((char *)0));
		if (canseemon(magr)) pline("A sinister darkness fills the area!");
		if (magr->data->mlet == S_ANGEL) tmp *= 2;
		break;

	    case AD_THIR:
	    case AD_NTHR:
		if (mdef->mhp > 0) {
		mdef->mhp += tmp;
		if (mdef->mhp > mdef->mhpmax) mdef->mhp = mdef->mhpmax;
		if (canseemon(mdef)) pline("%s looks healthier!", Monnam(mdef) );
		}
		break;

	    case AD_RAGN:		
		ragnarok(FALSE);
		if (evilfriday && mdef->m_lev > 1) evilragnarok(FALSE,mdef->m_lev);
		break;

	    case AD_AGGR:

		aggravate();
		if (!rn2(20)) {
			u.aggravation = 1;
			reset_rndmonst(NON_PM);
			(void) makemon((struct permonst *)0, mdef->mx, mdef->my, MM_ANGRY|MM_ADJACENTOK|MM_FRENZIED);
			u.aggravation = 0;
		}

		break;

	    case AD_CONT:

		if (!rn2(30)) {
			magr->isegotype = 1;
			magr->egotype_contaminator = 1;
		}
		if (!rn2(100)) {
			magr->isegotype = 1;
			magr->egotype_weeper = 1;
		}
		if (!rn2(250)) {
			magr->isegotype = 1;
			magr->egotype_radiator = 1;
		}
		if (!rn2(250)) {
			magr->isegotype = 1;
			magr->egotype_reactor = 1;
		}

		break;

	    case AD_FRZE:
		if (!resists_cold(magr) && resists_fire(magr)) {
			tmp *= 2;
			if (canseemon(magr)) pline("%s is suddenly ice-cold!", Monnam(magr));
		}
		break;
	    case AD_ICEB:
		if (!resists_cold(magr)) {
			tmp *= 2;
			if (canseemon(magr)) pline("%s is suddenly shockfrosted!", Monnam(magr));
		}
		break;
	    case AD_MALK:
		if (!resists_elec(magr)) {
			tmp *= 2;
			if (canseemon(magr)) pline("%s is jolted by high voltage!", Monnam(magr));
		}
		break;
	    case AD_UVUU:
		if (has_head(magr->data)) {
			tmp *= 2;
			if (!rn2(1000)) {
				tmp *= 100;
				if (canseemon(magr)) pline("%s's %s is torn apart!", Monnam(magr), mbodypart(magr, HEAD));
			} else if (canseemon(magr)) pline("%s's %s is spiked!", Monnam(magr), mbodypart(magr, HEAD));
		}
		break;
	    case AD_GRAV:
		if (!is_flyer(magr->data)) {
			tmp *= 2;
			if (canseemon(magr)) pline("%s slams into the ground!", Monnam(magr));
		}
		break;
	    case AD_CHKH:
		if (mdef->m_lev > magr->m_lev) tmp += (mdef->m_lev - magr->m_lev);
		break;
	    case AD_CHRN:
		if ((tmp > 0) && (magr->mhpmax > 1)) {
			magr->mhpmax--;
			if (canseemon(magr)) pline("%s feels bad!", Monnam(magr));
		}
		break;
	    case AD_HODS:
		tmp += magr->m_lev;
		break;
	    case AD_DIMN:
		tmp += mdef->m_lev;
		break;
	    case AD_BURN:
		if (resists_cold(magr) && !resists_fire(magr)) {
			tmp *= 2;
			if (canseemon(magr)) pline("%s is burning!", Monnam(magr));
		}
		break;
	    case AD_PLAS:
		if (!resists_fire(magr)) {
			tmp *= 2;
			if (canseemon(magr)) pline("%s is suddenly extremely hot!", Monnam(magr));
		}
		break;
	    case AD_SLUD:
		if (!resists_acid(magr)) {
			tmp *= 2;
			if (canseemon(magr)) pline("%s is covered with sludge!", Monnam(magr));
		}
		break;
	    case AD_LAVA:
		if (resists_cold(magr) && !resists_fire(magr)) {
			tmp *= 4;
			if (canseemon(magr)) pline("%s is scorched by hot lava!", Monnam(magr));
		} else if (!resists_fire(magr)) {
			tmp *= 2;
			if (canseemon(magr)) pline("%s is covered with hot lava!", Monnam(magr));
		}
		break;
	    case AD_FAKE:
		pline("%s", fauxmessage());
		if (!rn2(3)) pline("%s", fauxmessage());
		break;
	    case AD_WEBS:
		(void) maketrap(magr->mx, magr->my, WEB, 0);
		if (!rn2(issoviet ? 2 : 8)) makerandomtrap();
		break;
	    case AD_TRAP:
		if (t_at(magr->mx, magr->my) == 0) (void) maketrap(magr->mx, magr->my, randomtrap(), 0);
		else makerandomtrap();

		break;

	    case AD_CNCL:
		if (rnd(100) > magr->data->mr) {
			magr->mcan = 1;
			if (canseemon(magr)) pline("%s is covered in sparkling lights!", Monnam(magr));
		}
		break;
	    case AD_ICUR:
	    case AD_NACU:
	    case AD_CURS:
		if (!rn2(10) && (rnd(100) > magr->data->mr)) {
			magr->mcan = 1;
		}
		break;
	    case AD_FEAR:
		if (rnd(100) > magr->data->mr) {
		     monflee(magr, rnd(1 + tmp), FALSE, TRUE);
			if (canseemon(magr)) pline("%s is suddenly very afraid!",Monnam(magr));
		}
		break;
	    case AD_INSA:
		if (rnd(100) > magr->data->mr) {
		     monflee(magr, rnd(1 + tmp), FALSE, TRUE);
			if (canseemon(magr)) pline("%s is suddenly very afraid!",Monnam(magr));
		}
		if (!magr->mstun) {
		    magr->mstun = 1;
		    if (canseemon(magr))
			pline("%s %s...", Monnam(magr),
			      makeplural(stagger(magr->data, "stagger")));
		}
		if (!magr->mconf) {
		    if (canseemon(magr)) pline("%s is suddenly very confused!", Monnam(magr));
		    magr->mconf = 1;
		    magr->mstrategy &= ~STRAT_WAITFORU;
		}
		break;
	    case AD_SANI:
		if (!rn2(10)) {
			magr->mconf = 1;
			switch (rnd(4)) {

				case 1:
					pline("%s sees %s chow dead bodies.", Monnam(magr), mon_nam(mdef)); break;
				case 2:
					pline("%s shudders at %s's terrifying %s.", Monnam(magr), mon_nam(mdef), makeplural(mbodypart(mdef, EYE)) ); break;
				case 3:
					pline("%s feels sick at entrails caught in %s's tentacles.", Monnam(magr), mon_nam(mdef)); break;
				case 4:
					pline("%s sees maggots breed in the rent %s of %s.", Monnam(magr), mbodypart(mdef, STOMACH), mon_nam(mdef)); break;

			}

		}

		break;
	    case AD_DREA:
		if (!magr->mcanmove) {
			tmp *= 4;
			if (canseemon(magr)) pline("%s's dream is eaten!",Monnam(magr));
		}
		break;
	    case AD_CONF:
	    case AD_HALU:
	    case AD_DEPR:
	    case AD_SPC2:
		if (!magr->mconf) {
		    if (canseemon(magr)) pline("%s is suddenly very confused!", Monnam(magr));
		    magr->mconf = 1;
		    magr->mstrategy &= ~STRAT_WAITFORU;
		}
		break;

	    case AD_FAMN:
		if (magr->mtame) {
			makedoghungry(magr, tmp * rnd(50));
			if (canseemon(magr)) pline("%s suddenly looks hungry.", Monnam(magr));
		}
		break;

	    case AD_WRAT:
	    case AD_MANA:
	    case AD_TECH:
	    case AD_MEMO:
	    case AD_TRAI:
	    	    mon_drain_en(magr, ((magr->m_lev > 0) ? (rnd(magr->m_lev)) : 0) + 1 + tmp);
		break;
	    case AD_DREN:
	    	if (!resists_magm(magr)) {
	    	    mon_drain_en(magr, ((magr->m_lev > 0) ? (rnd(magr->m_lev)) : 0) + 1);
	    	}	    
		break;
	    case AD_BLND:
		    if (canseemon(magr) && magr->mcansee)
			pline("%s is blinded.", Monnam(magr));
		    if ((tmp += magr->mblinded) > 127) tmp = 127;
		    magr->mblinded = tmp;
		    magr->mcansee = 0;
		    magr->mstrategy &= ~STRAT_WAITFORU;
		tmp = 0;
		break;

	    case AD_PAIN:
		if (magr->mhp > 9) tmp += (magr->mhp / 10);
		if (canseemon(magr)) pline("%s shrieks in pain!", Monnam(magr));
		break;

	    case AD_DRLI:
	    case AD_TIME:
	    case AD_DFOO:
	    case AD_WEEP:
	    case AD_VAMP:
		if (!resists_drli(magr)) {
			if (canseemon(magr))
			    pline("%s suddenly seems weaker!", Monnam(magr));
			if (magr->m_lev == 0)
				tmp = magr->mhp;
			else magr->m_lev--;
			/* Automatic kill if drained past level 0 */
		}
		break;
	    case AD_VENO:
		if (resists_poison(magr)) {
			if (canseemon(magr))
			    pline_The("poison doesn't seem to affect %s.",
				mon_nam(magr));
		} else {
			if (canseemon(magr)) pline("%s is badly poisoned!", Monnam(magr));
			if (rn2(10)) tmp += rn1(20,12);
			else {
			    if (canseemon(magr)) pline_The("poison was deadly...");
			    tmp = magr->mhp;
			}
		}
		break;


	    default: /*tmp = 0;*/
		break;
	}
	else tmp = 0;

    assess_dmg:
	if((magr->mhp -= tmp) <= 0) {
		/* get experience from spell creatures */
		if (mdef->uexp) mon_xkilled(magr, "", (int)mddat->mattk[i].adtyp);
		else monkilled(magr, "", (int)mddat->mattk[i].adtyp);

		return (mdead | mhit | MM_AGR_DIED);
	}
	return (mdead | mhit);
}

/* "aggressive defense"; what type of armor prevents specified attack
   from touching its target? */
long
attk_protection(aatyp)
int aatyp;
{
    long w_mask = 0L;

    switch (aatyp) {
    case AT_NONE:
    case AT_RATH:
    case AT_SPIT:
    case AT_EXPL:
    case AT_BOOM:
    case AT_GAZE:
    case AT_BREA:
    case AT_MAGC:
    case AT_BEAM:
	w_mask = ~0L;		/* special case; no defense needed */
	break;
    case AT_CLAW:
    case AT_TUCH:
    case AT_WEAP:
	w_mask = W_ARMG;	/* caller needs to check for weapon */
	break;
    case AT_KICK:
	w_mask = W_ARMF;
	break;
    case AT_BUTT:
	w_mask = W_ARMH;
	break;
    case AT_HUGS:
	w_mask = (W_ARMC|W_ARMG); /* attacker needs both to be protected */
	break;
    case AT_BITE:
    case AT_STNG:
    case AT_LASH:
    case AT_TRAM:
    case AT_SCRA:
    case AT_ENGL:
    case AT_TENT:
    default:
	w_mask = 0L;		/* no defense available */
	break;
    }
    return w_mask;
}

STATIC_PTR void
set_lit(x,y,val)
int x, y;
void * val;
{
	if (val)
	    levl[x][y].lit = 1;
	else {
	    levl[x][y].lit = 0;
	    snuff_light_source(x, y);
	}
}

/* have the stooges say something funny */
STATIC_OVL void
stoogejoke()
{
	verbalize("%s", random_joke[rn2(SIZE(random_joke))]);
}

#endif /* OVLB */

/*mhitm.c*/

