/*	SCCS Id: @(#)u_init.c	3.4	2002/10/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

struct trobj {
	short trotyp;
	schar trspe;
	char trclass;
	Bitfield(trquan,6);
	Bitfield(trbless,2);
};

STATIC_DCL void FDECL(ini_inv, (struct trobj *));
STATIC_DCL void FDECL(knows_object,(int));
STATIC_DCL void FDECL(knows_class,(CHAR_P));
STATIC_DCL boolean FDECL(restricted_spell_discipline, (int));

#define UNDEF_TYP	0
#define UNDEF_SPE	'\177'
#define UNDEF_BLESS	2

/*
 *	Initial inventory for the various roles.
 */

static struct trobj Archeologist[] = {
#define A_BOOK          4
	/* if adventure has a name...  idea from tan@uvm-gen */
	{ BULLWHIP, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FEDORA, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },        
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 2, UNDEF_BLESS },
	{ PICK_AXE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ TOUCHSTONE, 0, GEM_CLASS, 1, 0 },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Artist[] = {
	{ SPOON, 2, TOOL_CLASS, 1, UNDEF_BLESS },
	{ TOUCHSTONE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ FEDORA, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ MALLET, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ GAUNTLETS_OF_DEXTERITY, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LAB_COAT, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 1, UNDEF_BLESS },
	{ CRAM_RATION, 0, FOOD_CLASS, 2, UNDEF_BLESS },
	{ POT_OIL, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ POT_WATER, 0, POTION_CLASS, 1, UNDEF_BLESS },
	{ MAGIC_MARKER, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Gamer[] = {
	{ T_SHIRT, 1, ARMOR_CLASS, 1, 1 },
	{ SPOON, UNDEF_SPE, WEAPON_CLASS, 13, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ CRAM_RATION, 0, FOOD_CLASS, 4, 1 },
	{ K_RATION, 0, FOOD_CLASS, 2, 1 },
	{ CANDY_BAR, 0, FOOD_CLASS, 7, 1 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Librarian[] = {
	{ ROBE, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LENSES, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ MAGIC_MARKER, 0, TOOL_CLASS, 1, UNDEF_BLESS }, /* empty on purpose */
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },        
	{ SPE_BLANK_PAPER, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },        
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, 1 },        
	{ SCR_BLANK_PAPER, UNDEF_SPE, SCROLL_CLASS, 5, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Supermarket[] = {
	{ KNIFE, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_JACKET, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LOW_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	{ TIN_OPENER, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ POT_FRUIT_JUICE, 0, POTION_CLASS, 3, UNDEF_BLESS },
	{ POT_BOOZE, 0, POTION_CLASS, 2, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Scribe[] = {
	{ MAGIC_MARKER, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS },        
	{ SCR_BLANK_PAPER, UNDEF_SPE, SCROLL_CLASS, 5, UNDEF_BLESS },
	{ DAGGER, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ CLOAK_OF_DISPLACEMENT, 0, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Augurer[] = {
#define AUG_BOOK          2
	{ LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ BLINDFOLD, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },        
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Barbarian[] = {
#define B_MAJOR	0	/* two-handed sword or battle-axe  */
#define B_MINOR	1	/* matched with axe or short sword */
	{ TWO_HANDED_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ AXE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ RING_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Binder[] = {
	{ VOULGE, 0, WEAPON_CLASS, 1, 0 },
	{ KNIFE, 0, WEAPON_CLASS, 1, 0 },
	{ ROCK, 0, GEM_CLASS, 5, 0 }, 
	{ FLINT, 0, GEM_CLASS, 1, 0 },
	{ LEATHER_CLOAK, 0, ARMOR_CLASS, 1, 0 },
	{ CRAM_RATION, 0, FOOD_CLASS, 1, 0 },
	{ APPLE, 0, FOOD_CLASS, 2, 0 },
	{ TRIPE_RATION, 0, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Politician[] = {
	{ T_SHIRT, 2, ARMOR_CLASS, 1, 0 },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, 0 },
	{ LOW_BOOTS, 0, ARMOR_CLASS, 1, 0 },
	{ BAG_OF_HOLDING, 0, TOOL_CLASS, 1, 0 },
	{ BELL, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Cook[] = {
	{ ROBE, 2, ARMOR_CLASS, 1, 0 },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 2, 0 },
	{ KNIFE, 2, WEAPON_CLASS, 1, 0 },
	{ LARGE_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS }, /* supposed to be a wok */
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Bleeder[] = {
	{ KNIFE, 0, WEAPON_CLASS, 1, 0 },
	{ POT_VAMPIRE_BLOOD, 0, POTION_CLASS, 2, 0 },
	{ LAB_COAT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ RIN_TIMELY_BACKUP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ AMULET_OF_DATA_STORAGE, 0, AMULET_CLASS, 1, UNDEF_BLESS },
	{ ICE_BOX, 0, TOOL_CLASS, 1, 0 },
	{ MIRROR, 0, TOOL_CLASS, 1, 0 },
	{ CAN_OF_GREASE, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ SCR_ROOT_PASSWORD_DETECTION, 0, SCROLL_CLASS, 1, 1 },
	{ SPE_PROTECTION, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_RESIST_POISON, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_BOTOX_RESIST, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ FAERIE_FLOSS_RHING, 0, VENOM_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Bard[] = {
#define BARD_INSTR 0
#define BARD_BOOZE 4
#define BARD_WHISTLE 5
	{ WOODEN_HARP, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_CLOAK, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 3, 0 },
	{ ORANGE, 0, FOOD_CLASS, 3, 0 },
	{ POT_BOOZE, 0, POTION_CLASS, 1, UNDEF_BLESS },
	{ TIN_WHISTLE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Cave_man[] = {
#define C_AMMO	2
	{ CLUB, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SLING, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ FLINT, 0, GEM_CLASS, 15, UNDEF_BLESS },	/* quan is variable */
	{ ROCK, 0, GEM_CLASS, 3, 0 },			/* yields 18..33 */
	{ LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Electric_Mage[] = {
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },        /* for dealing with ghosts */
	{ STUDDED_LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ WAN_LIGHTNING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ SPE_SHOCKING_SPHERE, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_LIGHTNING, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
  	{ 0, 0, 0, 0, 0 }
};

static struct trobj Abuser[] = {
	{ BULLWHIP, 8, WEAPON_CLASS, 1, 1 },
	{ KNOUT, 2, WEAPON_CLASS, 1, 1 },
	{ IRON_CHAIN, 8, CHAIN_CLASS, 1, 1 },
	{ VICTORIAN_UNDERWEAR, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_JACKET, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ KICKING_BOOTS, 3, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ AMULET_OF_POWER, 0, AMULET_CLASS, 1, UNDEF_BLESS },
	{ LEASH, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ SADDLE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ CAN_OF_GREASE, 50, TOOL_CLASS, 1, UNDEF_BLESS },
	{ SCR_PUNISHMENT, 0, SCROLL_CLASS, 1, 0 },
	{ SPE_GODMODE, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ WAN_PUNISHMENT, 6, WAND_CLASS, 1, 0 },
  	{ 0, 0, 0, 0, 0 }
};

static struct trobj Jester[] = {

	{ HAWAIIAN_SHIRT, 3, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ DUNCE_CAP, 3, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, GEM_CLASS, 27, UNDEF_BLESS },
	{ DAGGER, 1, WEAPON_CLASS, 2, UNDEF_BLESS },
	{ CREAM_PIE, 0, FOOD_CLASS, 10, UNDEF_BLESS },
	{ CORPSE, 0, FOOD_CLASS, 1, UNDEF_BLESS },
	{ RUBBER_HOSE, UNDEF_SPE, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ TOWEL, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, UNDEF_BLESS },

  	{ 0, 0, 0, 0, 0 }
};

static struct trobj Ladiesman[] = {
	{ KNIFE, UNDEF_SPE, WEAPON_CLASS, 2, UNDEF_BLESS },
	{ DIAMOND, UNDEF_SPE, GEM_CLASS, 3, UNDEF_BLESS },
	{ RIN_ADORNMENT, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS },
	{ RUFFLED_SHIRT, UNDEF_SPE, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FEDORA, UNDEF_SPE, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ SACK, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ LENSES, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },

  	{ 0, 0, 0, 0, 0 }
};

static struct trobj Bloodseeker[] = {

	{ SHORT_SWORD, UNDEF_SPE, WEAPON_CLASS, 1, 0 },
	{ COMBAT_STILETTOS, 2, ARMOR_CLASS, 1, 0 },
	{ WAN_FIRE, 2, WAND_CLASS, 1, 0 },

  	{ 0, 0, 0, 0, 0 }
};

static struct trobj Mystic[] = {

	{ UNDEF_TYP, UNDEF_SPE, AMULET_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, WEAPON_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, ARMOR_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, GEM_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, ROCK_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, BALL_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, CHAIN_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, VENOM_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, RANDOM_CLASS, 1, 0 },

  	{ 0, 0, 0, 0, 0 }
};

static struct trobj SlaveMaster[] = {
	{ TRIPE_RATION, 0, FOOD_CLASS, 3, 0 },
	{ FOOD_RATION, 0, FOOD_CLASS, 5, 0 },
	{ APPLE, 0, FOOD_CLASS, 3, 0 },
	{ ORANGE, 0, FOOD_CLASS, 1, 0 },
	{ TIN, 1, FOOD_CLASS, 2, 0 },
	{ LEASH, 2, TOOL_CLASS, 1, 0 },
	{ SADDLE, 0, TOOL_CLASS, 1, 0 },
	{ MAGIC_WHISTLE, 0, TOOL_CLASS, 1, 0 },

  	{ 0, 0, 0, 0, 0 }
};

static struct trobj Acid_Mage[] = {
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },        /* for dealing with ghosts */
	{ STUDDED_LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ WAN_ACID, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ SPE_ACID_SPHERE, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_ACID_STREAM, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
  	{ 0, 0, 0, 0, 0 }
};

static struct trobj Flame_Mage[] = {
#define F_BOOK          9
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },        /* for dealing with ghosts */
	{ STUDDED_LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ WAN_FIRE, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ SPE_FLAME_SPHERE, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_FIREBALL, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
  	{ 0, 0, 0, 0, 0 }
};
#ifdef CONVICT
static struct trobj Convict[] = {
	{ ROCK, 0, GEM_CLASS, 1, 0 },
	{ STRIPED_SHIRT, 0, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif  /* CONVICT */

static struct trobj Courier[] = {
	{ PISTOL, 0, WEAPON_CLASS, 1, 0 },
	{ BULLET, 0, WEAPON_CLASS, 20, 0 },
	{ KNIFE, 0, WEAPON_CLASS, 1, 0 },
	{ POT_JOLT_COLA, 0, POTION_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Spacewars_Fighter[] = {
	{ DAGGER, 0, WEAPON_CLASS, 1, 0 },
	{ SACK, 0, TOOL_CLASS, 1, 0 }, /* since there is no "small pack" in NetHack --Amy */
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Rocker[] = {
	{ ROCK, UNDEF_SPE, GEM_CLASS, 1, UNDEF_BLESS },
	{ ROCK, UNDEF_SPE, GEM_CLASS, 1, UNDEF_BLESS },
	{ ROCK, UNDEF_SPE, GEM_CLASS, 1, UNDEF_BLESS },
	{ ROCK, UNDEF_SPE, GEM_CLASS, 1, UNDEF_BLESS },
	{ ROCK, UNDEF_SPE, GEM_CLASS, 1, UNDEF_BLESS },
	{ FLINT, UNDEF_SPE, GEM_CLASS, 1, UNDEF_BLESS },
	{ SLING, 2, WEAPON_CLASS, 1, 1 },
	{ WOODEN_HARP, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ PICK_AXE, 1, TOOL_CLASS, 1, 1 },
	{ SPE_FORCE_BOLT, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Zyborg[] = {
	{ PLATE_MAIL, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ GREEN_LIGHTSABER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DIODE, UNDEF_SPE, TOOL_CLASS, 20, UNDEF_BLESS },
	{ WAN_SOLAR_BEAM, UNDEF_SPE, WAND_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Lunatic[] = {
	{ HELM_OF_STEEL, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ GAUNTLETS_OF_STEEL, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ STEEL_SHIELD, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ KICKING_BOOTS, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ STEEL_WHIP, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ TRIPE_RATION, 0, FOOD_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Drunk[] = {
	{ RUBBER_HOSE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },    /* quan is variable */
	{ POT_BOOZE, UNDEF_SPE, POTION_CLASS, 5, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 5, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 5, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 5, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, }
};

static struct trobj Officer[] = {
	{ PISTOL, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ CLUB, 0, WEAPON_SYM, 1, UNDEF_BLESS },
	{ LEATHER_ARMOR, 0, ARMOR_SYM, 1, UNDEF_BLESS },
	{ WAN_MAGIC_MISSILE, UNDEF_SPE, WAND_SYM, 1, UNDEF_BLESS },
	{ WAN_STRIKING, UNDEF_SPE, WAND_SYM, 1, UNDEF_BLESS },
	{ HACKER_S_FOOD, 0, FOOD_SYM, 6, 0 },
	{ 0, 0, 0, 0, 0, }
};

static struct trobj Musician[] = {
	{ FEDORA, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HAWAIIAN_SHIRT, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ MAGIC_FLUTE, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ MAGIC_HARP, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ DRUM_OF_EARTHQUAKE, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ FROST_HORN, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ FIRE_HORN, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, }
};

static struct trobj Undertaker[] = {
	{ SCALPEL, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ VICTORIAN_UNDERWEAR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ WAN_UNDEAD_TURNING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ WAN_UNDEAD_TURNING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, }
};

static struct trobj Zookeeper[] = {
	{ BULLWHIP, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ TRIPE_RATION, 0, TOOL_CLASS, 5, UNDEF_BLESS },
	{ SCR_TAMING, UNDEF_SPE, SCROLL_CLASS, 3, 1 },
	{ LEASH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0, }
};
static struct trobj Firefighter[] = {
	{ AXE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ POT_WATER, 0, POTION_CLASS, 5, 0 },
	{ SCR_FIRE, UNDEF_SPE, SCROLL_CLASS, 4, UNDEF_BLESS },
	{ WAN_FIRE, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, }
};

static struct trobj Locksmith[] = {
	{ WAN_OPENING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ WAN_LOCKING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ SKELETON_KEY, 0, TOOL_CLASS, 1, 0 },
	{ LOCK_PICK, 9, TOOL_CLASS, 1, 0 },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0, }
};

static struct trobj Ninja[] = {
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },      /* the wakizashi */
	{ SHURIKEN, 0, WEAPON_CLASS, 9, UNDEF_BLESS },        /* quan is variable */
	{ POT_SICKNESS, 0, POTION_CLASS, 1, 0 },
	{ POT_INVISIBILITY, UNDEF_SPE, POTION_CLASS, 1, 1 },
	{ LOCK_PICK, 9, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0, }
};

static struct trobj Gangster[] = {
	{ SUBMACHINE_GUN, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Gunner[] = {
	{ ASSAULT_RIFLE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ PISTOL, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SNIPER_RIFLE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, 0 },
	{ MEDICAL_KIT, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Death_Eater[] = {
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },
	{ SHOTGUN, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SHOTGUN_SHELL, 0, WEAPON_CLASS, 10, 0 },
	{ CLOAK_OF_REFLECTION, 1, ARMOR_CLASS, 1, 0 },
	{ SPE_FINGER_OF_DEATH, UNDEF_SPE, SPBOOK_CLASS, 1, 1},
	{ SPE_CHARM_MONSTER, UNDEF_SPE, SPBOOK_CLASS, 1, 1},
	{ SPE_ACID_STREAM, UNDEF_SPE, SPBOOK_CLASS, 1, 1},
	{ WAN_DEATH, UNDEF_SPE, WAND_CLASS, 1, 0 },
	{ AMULET_OF_LIFE_SAVING, UNDEF_SPE, AMULET_CLASS, 1, 0 },
	{ TIN, UNDEF_SPE, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Pokemon[] = {
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 5, 0 },
	{ WAN_ENLIGHTENMENT, UNDEF_SPE, WAND_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};


static struct trobj Geek[] = {
	{ ELECTRIC_SWORD, 1, WEAPON_CLASS, 1, 1},
#define G_IC 1
	{ IC, 0, TOOL_CLASS, 7, UNDEF_BLESS },	/* quan is var. */
	{ GAUNTLETS_OF_TYPING, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_JOLT_COLA, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ HACKER_S_FOOD, 0, FOOD_CLASS, 2, UNDEF_BLESS },
	{ PACK_OF_FLOPPIES, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Graduate[] = {
	{ SPOON, 3, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ CANDY_BAR, 0, FOOD_CLASS, 5, 0 },
	{ EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Scientist[] = {
	{ SCALPEL, 1, WEAPON_CLASS, 1, 1},
	{ GAUNTLETS_OF_FUMBLING, 5, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LAB_COAT, 5, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_RADIUM, 0, POTION_CLASS, 1, UNDEF_BLESS },
	{ POT_CYANIDE, 0, POTION_CLASS, 1, UNDEF_BLESS },
	{ POT_PARALYSIS, 0, POTION_CLASS, 1, UNDEF_BLESS },
	{ POT_ACID, 0, POTION_CLASS, 1, UNDEF_BLESS },
	{ POT_SICKNESS, 0, POTION_CLASS, 1, UNDEF_BLESS },
	{ BOTTLE, 0, TOOL_CLASS, 3, 0 },
	{ HACKER_S_FOOD, 0, FOOD_CLASS, 2, UNDEF_BLESS },
	{ SPE_CHEMISTRY, 5, SPBOOK_CLASS, 1, UNDEF_BLESS },
	{ CHEMISTRY_SET, 40, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Healer[] = {
	{ SCALPEL, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ STETHOSCOPE, 0, TOOL_CLASS, 1, 0 },
	{ MEDICAL_KIT, 0, TOOL_CLASS, 1, 0 },        
	{ POT_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ POT_EXTRA_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ WAN_SLEEP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	/* [Tom] they might as well have a wand of healing, too */        
	{ WAN_HEALING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	/* always blessed, so it's guaranteed readable */
	{ SPE_HEALING, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_STONE_TO_FLESH, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Ice_Mage[] = {
#define I_BOOK          9
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },        /* for dealing with ghosts */
	{ STUDDED_LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ WAN_COLD, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ SPE_FREEZE_SPHERE, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_CONE_OF_COLD, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef JEDI
static struct trobj Jedi[] = {
	{ ROBE, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj GreenSaber[] = {
	{ GREEN_LIGHTSABER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, }
};
#ifdef D_SABER
static struct trobj BlueSaber[] = {
	{ BLUE_LIGHTSABER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, }
};
#endif
static struct trobj RedSaber[] = {
	{ RED_LIGHTSABER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, }
};
#endif
static struct trobj Knight[] = {
	{ LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LANCE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ PLATE_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LARGE_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ CARROT, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Wandkeeper[] = {
	{ KNIFE, 1, WEAPON_CLASS, 1, 0 },
	{ LAB_COAT, 1, ARMOR_CLASS, 1, 1 },
	{ SCR_CHARGING, 0, SCROLL_CLASS, 1, 1 },
	{ SPE_POLYMORPH, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ WAN_MAGIC_MISSILE, 16, WAND_CLASS, 2, 1 },
	{ UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 5, 1 },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Paladin[] = {
	{ LONG_SWORD, 3, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LARGE_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ STUDDED_LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LOW_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 4, UNDEF_BLESS },
	{ RIN_WARNING, 0, RING_CLASS, 1, UNDEF_BLESS },

	{ 0, 0, 0, 0, 0 }
};

static struct trobj FoxhoundAgent[] = {
	{ FEDORA, 0, ARMOR_CLASS, 1, 1 },
	{ KICKING_BOOTS, 0, ARMOR_CLASS, 1, 0 },
	{ ELVEN_CLOAK, 2, ARMOR_CLASS, 1, 1 },
	{ LEATHER_GLOVES, 1, ARMOR_CLASS, 1, 0 },
	{ LEMBAS_WAFER, 0, FOOD_CLASS, 2, UNDEF_BLESS },
	{ K_RATION, 0, FOOD_CLASS, 3, UNDEF_BLESS },
	{ ASIAN_PEAR, 0, FOOD_CLASS, 10, 0 },
	{ KNIFE, 2, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj StuntMaster[] = {
	{ FLAIL, 1, WEAPON_CLASS, 1, 0 },
	{ ORCISH_HELM, 0, ARMOR_CLASS, 1, 0 },
	{ LEATHER_JACKET, 2, ARMOR_CLASS, 1, 0 },
	{ CANDY_BAR, 0, FOOD_CLASS, 5, 0 },
	{ POT_HEALING, 0, POTION_CLASS, 2, 0 },
	{ EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Warrior[] = {
	{ MACE, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ CROSSBOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ CROSSBOW_BOLT, 0, WEAPON_CLASS, 50, 1 },
	{ LARGE_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 15, 0 },
	{ CARROT, 0, FOOD_CLASS, 15, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj MahouShoujo[] = {

	{ UNDEF_TYP, 0, ARMOR_CLASS, 1, 0 },
	{ UNDEF_TYP, 0, ARMOR_CLASS, 1, 0 },
	{ WAN_STRIKING, UNDEF_SPE, WAND_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 3, 0 },
	{ TIN, UNDEF_SPE, FOOD_CLASS, 3, 0 },

	{ 0, 0, 0, 0, 0 }
};

static struct trobj DollMistress[] = {
	{ WORM_TOOTH, 1, WEAPON_CLASS, 1, 0 },
	{ DWARVISH_CLOAK, 2, ARMOR_CLASS, 1, 0 },
	{ VICTORIAN_UNDERWEAR, 0, ARMOR_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 12, 0 },
	{ CRAM_RATION, 0, FOOD_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, 0 },
	{ UNDEF_TYP, 0, POTION_CLASS, 1, 0 },
	{ WAX_CANDLE, 0, TOOL_CLASS, 7, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Monk[] = {
#define M_BOOK		2
	{ LEATHER_GLOVES, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ ROBE, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ POT_HEALING, 0, POTION_CLASS, 3, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ APPLE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	{ ORANGE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	/* Yes, we know fortune cookies aren't really from China.  They were
	 * invented by George Jung in Los Angeles, California, USA in 1916.
	 */
	{ FORTUNE_COOKIE, 0, FOOD_CLASS, 3, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Saiyan[] = {
	{ ROBE, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Psion[] = {
#define PS_BOOK		2
	{ LEATHER_GLOVES, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ ROBE, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ POT_MONSTER_DETECTION, 0, POTION_CLASS, 3, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ APPLE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	{ ORANGE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	/* Yes, we know fortune cookies aren't really from China.  They were
	 * invented by George Jung in Los Angeles, California, USA in 1916.
	 */
	{ FORTUNE_COOKIE, 0, FOOD_CLASS, 3, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Noble[] = {
#define NOB_SHIRT	1
	{ RAPIER, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ RUFFLED_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_CLOAK, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Pirate[] = {
#define PIR_KNIVES	1
#define PIR_SNACK 5
#define PIR_JEWELRY 7
#define PIR_TOOL 8
	{ SCIMITAR, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ KNIFE, 1, WEAPON_CLASS, 2, 0 },
	{ PISTOL, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CRAM_RATION, 0, FOOD_CLASS, 2, UNDEF_BLESS },
	{ BANANA, 0, FOOD_CLASS, 3, 0 },
	{ POT_BOOZE, 0, POTION_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ OILSKIN_SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Korsair[] = {
	{ SCIMITAR, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DAGGER, 1, WEAPON_CLASS, 7, 0 },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ AMULET_OF_MAGICAL_BREATHING, 0, AMULET_CLASS, 1, 0 },
	{ OILSKIN_SACK, 0, TOOL_CLASS, 1, 0 },
	{ POT_BOOZE, 0, POTION_CLASS, 2, UNDEF_BLESS },
	{ TIN, 0, FOOD_CLASS, 1, 0 },
	{ CRAM_RATION, 0, FOOD_CLASS, 1, 1 },
	{ FIGURINE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Gladiator[] = {
	{ SILVER_SPEAR, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SHIELD_OF_REFLECTION, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CRAM_RATION, 0, FOOD_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Altmer[] = {
	{ QUARTERSTAFF, 2, WEAPON_CLASS, 1, 1 },
	{ ROBE, 2, ARMOR_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 2, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 2, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Bosmer[] = {
	{ ELVEN_DAGGER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_BOW, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_ARROW, 4, WEAPON_CLASS, 50, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Dunmer[] = {
	{ DARK_ELVEN_DAGGER, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DARK_ELVEN_BOW, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DARK_ELVEN_ARROW, 0, WEAPON_CLASS, 50, 0 },
	{ DARK_ELVEN_ARROW, 0, WEAPON_CLASS, 50, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Ordinator[] = {
	{ TSURUGI, 3, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ RUNESWORD, 3, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ MALLET, 4, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DROVEN_CROSSBOW, 3, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DROVEN_BOLT, 8, WEAPON_CLASS, 50, UNDEF_BLESS },
	{ RIN_INCREASE_ACCURACY, 5, RING_CLASS, 1, UNDEF_BLESS },
	{ SPE_ENCHANT_WEAPON, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Thalmor[] = {
	{ ELVEN_BROADSWORD, UNDEF_SPE, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_MITHRIL_COAT, UNDEF_SPE, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ DARK_ELVEN_BOW, UNDEF_SPE, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DARK_ELVEN_ARROW, UNDEF_SPE, WEAPON_CLASS, 50, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Amazon[] = {
	{ JAVELIN, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BOW, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ARROW, 0, WEAPON_CLASS, 50, 1 },
	{ ARROW, 0, WEAPON_CLASS, 50, 1 },
	{ SHIELD_OF_MOBILITY, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_ARMOR, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 1 },
	{ POT_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ SCR_TELEPORTATION, 0, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Goff[] = {
	{ PISTOL, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ HIPPIE_HEELS, 5, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ VICTORIAN_UNDERWEAR, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ BLACK_DRAGON_SCALE_MAIL, -9, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_ACID, 0, POTION_CLASS, 3, UNDEF_BLESS },
	{ POT_VAMPIRE_BLOOD, 0, POTION_CLASS, 5, UNDEF_BLESS },
	{ POT_BLOOD, 0, POTION_CLASS, 5, UNDEF_BLESS },
	{ AMULET_OF_COVETOUS_WARNING, 0, AMULET_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Diver[] = {
	{ KNIFE, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SPEAR, 0, WEAPON_CLASS, 3, UNDEF_BLESS },
	{ TRIDENT, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ OILSKIN_CLOAK, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CAN_OF_GREASE, UNDEF_SPE, TOOL_CLASS, 1, 1 },
	{ TALLOW_CANDLE, 0, TOOL_CLASS, 3, 1 },
	{ AMULET_OF_MAGICAL_BREATHING, 0, AMULET_CLASS, 1, 0 },
	{ TOWEL, 0, TOOL_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Necromancer[] = {
#define N_BOOK          8
/* pretty much like Wizard, except with pick-axe instead of magic resist. */
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },        /* for dealing with ghosts */
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS },
	{ SPE_SUMMON_UNDEAD, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_COMMAND_UNDEAD, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_DRAIN_LIFE, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
		/* WAC -- gave him drain life rather than turn undead */
	{ WAN_DRAINING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
		/* KMH -- ...and the matching wand for the inexperienced */
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ PICK_AXE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Chevalier[] = {
	{ TRIDENT, 0, WEAPON_CLASS, 1, 1 },
	{ ROBE, 0, ARMOR_CLASS, 1, 1 },
	{ SMALL_SHIELD, 2, ARMOR_CLASS, 1, 1 },
	{ POT_WATER, 0, POTION_CLASS, 2, 1 },	/* holy water */
	{ CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 1 },
	{ SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 1 },
	{ SPE_HEALING, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, 1 },
	{ SADDLE, 0, TOOL_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Priest[] = {
#define P_BOOK          7
	{ MACE, 0, WEAPON_CLASS, 1, 1 },
	{ ROBE, 0, ARMOR_CLASS, 1, 1 },
	{ SMALL_SHIELD, 2, ARMOR_CLASS, 1, 1 },
	{ POT_WATER, 0, POTION_CLASS, 4, 1 },	/* holy water */
	{ CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 1 },
	{ SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 1 },
	{ SPE_HEALING, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Ranger[] = {
#define RAN_BOW			1
#define RAN_TWO_ARROWS	2
#define RAN_ZERO_ARROWS	3
	{ DAGGER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ARROW, 2, WEAPON_CLASS, 50, UNDEF_BLESS },
	{ ARROW, 0, WEAPON_CLASS, 30, UNDEF_BLESS },
	{ CLOAK_OF_DISPLACEMENT, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CRAM_RATION, 0, FOOD_CLASS, 4, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Elph[] = {
#define ELP_BOW			1
#define ELP_TWO_ARROWS	2
#define ELP_ZERO_ARROWS	3
	{ DAGGER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_ARROW, 2, WEAPON_CLASS, 50, UNDEF_BLESS },
	{ ELVEN_ARROW, 0, WEAPON_CLASS, 30, UNDEF_BLESS },
	{ CLOAK_OF_DISPLACEMENT, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CRAM_RATION, 0, FOOD_CLASS, 4, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Assassin[] = {

	{ BOW, 1, WEAPON_CLASS, 1, 0 },
	{ ARROW, 2, WEAPON_CLASS, 50, 0 },
	{ DAGGER, 1, WEAPON_CLASS, 2, 0 },
	{ LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_CLOAK, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 4, 0 },
	{ POT_PARALYSIS, 0, POTION_CLASS, 1, 0 },
	{ POT_SLEEPING, 0, POTION_CLASS, 1, 0 },

	{ 0, 0, 0, 0, 0 }
};

static struct trobj Bully[] = {

	{ CLUB, 0, WEAPON_CLASS, 1, 0 },
	{ KNIFE, 0, WEAPON_CLASS, 1, 0 },
	{ DUNCE_CAP, 3, ARMOR_CLASS, 1, 0 },
	{ T_SHIRT, 3, ARMOR_CLASS, 1, 0 },
	{ CANDY_BAR, 0, FOOD_CLASS, 10, 0 },

	{ 0, 0, 0, 0, 0 }
};

static struct trobj Pickpocket[] = {

	{ DAGGER, 0, WEAPON_CLASS, 10, 0 },
	{ KNIFE, 0, WEAPON_CLASS, 2, 0 },
	{ LOCK_PICK, 0, TOOL_CLASS, 1, 0 },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CLOAK_OF_DISPLACEMENT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },

	{ 0, 0, 0, 0, 0 }
};

static struct trobj Rogue[] = {
#define R_DAGGERS	1
#define R_DARTS         2
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DAGGER, 0, WEAPON_CLASS, 10, 0 },	/* quan is variable */
	{ DART, 0, WEAPON_CLASS, 25, UNDEF_BLESS },
	{ LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_SICKNESS, 0, POTION_CLASS, 1, 0 },
	{ SCR_GOLD_DETECTION, 0, SCROLL_CLASS, 4, 1 },
	{ SCR_TELEPORTATION, 0, SCROLL_CLASS, 4, 1 },
	{ LOCK_PICK, 9, TOOL_CLASS, 1, 0 },
	{ OILSKIN_SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Samurai[] = {
#define S_ARROWS	3
	{ KATANA, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS }, /* wakizashi */
	{ YUMI, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ YA, 0, WEAPON_CLASS, 25, UNDEF_BLESS }, /* variable quan */
	{ SPLINT_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Transvestite[] = {
	{ WEDGED_LITTLE_GIRL_SANDAL, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ WEDGE_SANDALS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ SADDLE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Topmodel[] = {
	{ KNIFE, 1, WEAPON_CLASS, 1, 1 },
	{ RIFLE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ LEATHER_PEEP_TOES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ SADDLE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Activistor[] = {
#define ACT_SHIRT	0
	{ RUFFLED_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FLY_SWATTER, 1, WEAPON_CLASS, 1, 1 },
	{ SPE_CHARM_MONSTER, UNDEF_SPE, SPBOOK_CLASS, 1, 1},
	{ EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Otaku[] = {
	{ KATANA, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ T_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 9, 0 },
	{ UNDEF_TYP, 0, POTION_CLASS, 2, UNDEF_BLESS },
	{ POT_WATER, 0, POTION_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, 0, SCROLL_CLASS, 3, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};


#ifdef TOURIST
static struct trobj Tourist[] = {
#define T_DARTS		0
	{ DART, 2, WEAPON_CLASS, 25, UNDEF_BLESS },	/* quan is variable */
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 12, 0 },
	{ POT_EXTRA_HEALING, 0, POTION_CLASS, 2, UNDEF_BLESS },
	{ SCR_MAGIC_MAPPING, 0, SCROLL_CLASS, 6, UNDEF_BLESS },
	{ HAWAIIAN_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ CREDIT_CARD, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif
static struct trobj UndeadSlayer[] = {
#define U_MINOR 1       /* silver spear or whip [Castlevania] 25/25% */
                        /* crossbow 50% [Buffy] */
#define U_RANGE 2       /* silver daggers or crossbow bolts */
#define U_MISC  3       /* +1 boots [Buffy can kick] or helmet */
#define U_ARMOR 4       /* Tshirt/leather +1 or chain mail */
	{ WOODEN_STAKE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SILVER_SPEAR, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SILVER_DAGGER, 0, WEAPON_CLASS, 5, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CHAIN_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CLOVE_OF_GARLIC, 0, FOOD_CLASS, 5, 1 },
	{ SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 5, 1 },
	{ HOLY_WAFER, 0, FOOD_CLASS, 4, 0 },
	{ POT_WATER, 0, POTION_CLASS, 4, 1 },        /* holy water */
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Valkyrie[] = {
/*        { LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS }, */
	{ SPEAR, 1, WEAPON_CLASS, 1, UNDEF_BLESS }, /* Traditional Weapon - WAC*/
	{ DAGGER, 0, WEAPON_CLASS, 5, UNDEF_BLESS },
	{ SMALL_SHIELD, 3, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Wizard[] = {
#define W_MULTSTART	2
#define W_MULTEND	6
#define W_BOOK1         6
#define W_BOOK2         7
#define W_BOOK3         8
#define W_BOOK4         9
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },
	{ CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Sage[] = {

	{ MACE, 1, WEAPON_CLASS, 1, 1 },
	{ ROBE, 2, ARMOR_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 2, UNDEF_BLESS },
	{ SPE_FORCE_BOLT, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_HEALING, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 2, 1 },
	{ POT_HEALING, UNDEF_SPE, POTION_CLASS, 2, 1 },
	{ 0, 0, 0, 0, 0 }
};


#ifdef YEOMAN
static struct trobj Yeoman[] = {
	{ SHORT_SWORD, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ PARTISAN, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ CARROT, 0, FOOD_CLASS, 10, 0 },
	{ POT_WATER, 0, POTION_CLASS, 3, 0 },
	{ FISHING_POLE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif

/*
 *	Optional extra inventory items.
 */

static struct trobj AlienItem[] = {
	{ LOADSTONE, 0, GEM_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj KopItemA[] = {
	{ RUBBER_HOSE, UNDEF_SPE, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj KopItemB[] = {
	{ CREAM_PIE, 0, FOOD_CLASS, 0, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj NaviItem[] = {
	{ WEDGE_SANDALS, 0, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj InsectoidItem[] = {
	{ LUMP_OF_ROYAL_JELLY, 0, FOOD_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj BloodseekerA[] = {
	{ SCR_TELEPORTATION, 0, SCROLL_CLASS, 1, UNDEF_SPE },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj SlaveMasterA[] = {
	{ OIL_LAMP, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj SlaveMasterB[] = {
	{ SCR_TAMING, 0, SCROLL_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj SlaveMasterC[] = {
	{ RIN_CONFLICT, 0, RING_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj SupermarketA[] = {
	{ OILSKIN_SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj SupermarketB[] = {
	{ OIL_LAMP, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj KoboltItem[] = {
	{ DART, 0, WEAPON_CLASS, 15, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj KoboltItemB[] = {
	{ ORCISH_DAGGER, 0, WEAPON_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj KoboltItemC[] = {
	{ ORCISH_SPEAR, 0, WEAPON_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj GhastFood[] = {
	{ CORPSE, 0, FOOD_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UngMoldWand[] = {
	{ WAN_DEATH, 10, WAND_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj MaiaWand[] = {
	{ WAN_TELEPORTATION, 12, WAND_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj AutomatonItem[] = {
	{ POT_OIL, 0, POTION_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};


static struct trobj TrollItemA[] = {
	{ RANSEUR, 0, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj TrollItemB[] = {
	{ PARTISAN, 0, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj TrollItemC[] = {
	{ GLAIVE, 0, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj TrollItemD[] = {
	{ SPETUM, 0, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj LostSoulItem[] = {
	{ FOOD_RATION, 0, FOOD_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj LostSoulItemX[] = {
	{ SCR_IDENTIFY, 0, SCROLL_CLASS, 15, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj LostSoulItemY[] = {
	{ TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj AlwaysStartItem[] = {
	{ SCR_HEALING, 0, SCROLL_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj InvisibilityRing[] = {
	{ RIN_INVISIBILITY, 0, RING_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj AlwaysStartItemB[] = {
	{ SCR_STANDARD_ID, 0, SCROLL_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj AlwaysStartItemC[] = {
	{ SCR_MANA, 0, SCROLL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj AlwaysStartItemD[] = {
	{ SCR_CURE, 0, SCROLL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj AngbandItem[] = {
	{ SCR_IDENTIFY, 0, SCROLL_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj AngbandItemB[] = {
	{ SCR_TELEPORTATION, 0, SCROLL_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemA[] = {
	{ TALLOW_CANDLE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemB[] = {
	{ POT_FRUIT_JUICE, 0, POTION_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj BullyA[] = {
	{ POT_FRUIT_JUICE, 0, POTION_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj BullyB[] = {
	{ POT_BOOZE, 0, POTION_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj BullyD[] = {
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj BullyE[] = {
	{ TIN_WHISTLE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemC[] = {
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemD[] = {
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemE[] = {
	{ UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemF[] = {
	{ UNDEF_TYP, UNDEF_SPE, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemG[] = {
	{ UNDEF_TYP, UNDEF_SPE, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemH[] = {
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemI[] = {
	{ UNDEF_TYP, UNDEF_SPE, AMULET_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemJ[] = {
	{ UNDEF_TYP, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemK[] = {
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemL[] = {
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemM[] = {
	{ UNDEF_TYP, UNDEF_SPE, GEM_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemN[] = {
	{ UNDEF_TYP, UNDEF_SPE, ROCK_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemO[] = {
	{ UNDEF_TYP, UNDEF_SPE, BALL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemP[] = {
	{ UNDEF_TYP, UNDEF_SPE, CHAIN_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemQ[] = {
	{ UNDEF_TYP, UNDEF_SPE, VENOM_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj UberLostSoulItemR[] = {
	{ UNDEF_TYP, UNDEF_SPE, RANDOM_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj GigantItem[] = {
	{ BOULDER, 0, ROCK_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj OgroItem[] = {
	{ CLUB, 0, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj BatmanItem[] = {
	{ BOOMERANG, 0, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Tinopener[] = {
	{ TIN_OPENER, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj XtraRing[] = {
	{ UNDEF_TYP, 0, RING_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Magicmarker[] = {
	{ MAGIC_MARKER, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Touchstone[] = {
	{ TOUCHSTONE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Candle[] = {
	{ TALLOW_CANDLE, 3, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Lamp[] = {
	{ OIL_LAMP, 1, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Torch[] = {
	{ TORCH, 0, TOOL_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Blindfold[] = {
	{ BLINDFOLD, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Instrument[] = {
	{ WOODEN_FLUTE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Xtra_food[] = {
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Xtra_fopod[] = {
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef TOURIST
static struct trobj Leash[] = {
	{ LEASH, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Towel[] = {
	{ TOWEL, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif	/* TOURIST */
static struct trobj Wishing[] = {
	{ WAN_WISHING, 3, WAND_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef GOLDOBJ
static struct trobj Money[] = {
	{ GOLD_PIECE, 0 , COIN_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

#endif

static struct trobj LichOne[] = {
	{ SPE_SUMMON_UNDEAD, 3, SPBOOK_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj LichTwo[] = {
	{ ATHAME, UNDEF_SPE, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj WraithItem[] = {
	{ SPE_PASSWALL, 10, SPBOOK_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj WarXtrA[] = {
	{ PLATE_MAIL, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrB[] = {
	{ PLASTEEL_ARMOR, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrC[] = {
	{ CRYSTAL_PLATE_MAIL, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrD[] = {
	{ BRONZE_PLATE_MAIL, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrE[] = {
	{ SPLINT_MAIL, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrF[] = {
	{ BANDED_MAIL, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrG[] = {
	{ CHAIN_MAIL, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrH[] = {
	{ SCALE_MAIL, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrI[] = {
	{ RING_MAIL, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrJ[] = {
	{ YELLOW_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrK[] = {
	{ GREEN_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrL[] = {
	{ BLUE_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrM[] = {
	{ BLACK_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrN[] = {
	{ ORANGE_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrO[] = {
	{ WHITE_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrP[] = {
	{ RED_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrQ[] = {
	{ DEEP_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrR[] = {
	{ SHIMMERING_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrS[] = {
	{ SILVER_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj WarXtrT[] = {
	{ GRAY_DRAGON_SCALES, 1, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};


/* race-based substitutions for initial inventory;
   the weaker cloak for elven rangers is intentional--they shoot better */
static struct inv_sub { short race_pm, item_otyp, subs_otyp; } inv_subs[] = {
    { PM_ELF,	DAGGER,			ELVEN_DAGGER	      },
    { PM_ELF,	SPEAR,			ELVEN_SPEAR	      },
    { PM_ELF,	SHORT_SWORD,		ELVEN_SHORT_SWORD     },
    { PM_ELF,	BOW,			ELVEN_BOW	      },
    { PM_ELF,	ARROW,			ELVEN_ARROW	      },
    { PM_ELF,	HELMET,			ELVEN_LEATHER_HELM    },
 { PM_ELF,	SMALL_SHIELD,		ELVEN_SHIELD	      },
    { PM_ELF,	CLOAK_OF_DISPLACEMENT,	ELVEN_CLOAK	      },
    { PM_ELF,	CRAM_RATION,		LEMBAS_WAFER	      },
    { PM_ORC,	DAGGER,			ORCISH_DAGGER	      },
    { PM_ORC,	SPEAR,			ORCISH_SPEAR	      },
    { PM_ORC,	SHORT_SWORD,		ORCISH_SHORT_SWORD    },
    { PM_ORC,	BOW,			ORCISH_BOW	      },
    { PM_ORC,	ARROW,			ORCISH_ARROW	      },
    { PM_ORC,	HELMET,			ORCISH_HELM	      },
    { PM_ORC,	SMALL_SHIELD,		ORCISH_SHIELD	      },
    { PM_ORC,	RING_MAIL,		ORCISH_RING_MAIL      },
    { PM_ORC,	CHAIN_MAIL,		ORCISH_CHAIN_MAIL     },
    { PM_DWARF, SPEAR,			DWARVISH_SPEAR	      },
    { PM_DWARF, SHORT_SWORD,		DWARVISH_SHORT_SWORD  },
    { PM_DWARF, HELMET,			DWARVISH_IRON_HELM    },
 { PM_DWARF, SMALL_SHIELD,		DWARVISH_ROUNDSHIELD  },
 { PM_DWARF, PICK_AXE,		DWARVISH_MATTOCK      },
    { PM_GNOME, BOW,			CROSSBOW	      },
    { PM_GNOME, ARROW,			CROSSBOW_BOLT	      },
    { PM_HUMAN_WEREWOLF,FOOD_RATION,		TRIPE_RATION	      },
    { PM_HUMAN_WEREWOLF,SILVER_SPEAR,		SPEAR		      },
    { PM_HUMAN_WEREWOLF,SILVER_DAGGER,		DAGGER		      },
#ifdef FIREARMS
    { PM_HUMAN_WEREWOLF,SILVER_BULLET,		BULLET		      },
#endif
    { PM_DROW,		DAGGER,			DARK_ELVEN_DAGGER     },
    { PM_DROW,		SHORT_SWORD,		DARK_ELVEN_SHORT_SWORD},
    { PM_DROW,		BOW,			DARK_ELVEN_BOW	      },
    { PM_DROW,		ARROW,			DARK_ELVEN_ARROW      },
    { PM_DROW,		CROSSBOW,			DROVEN_CROSSBOW      },
    { PM_DROW,		CROSSBOW_BOLT,			DROVEN_BOLT      },
	 { PM_GNOME, HELMET,	GNOMISH_HELM },
	 { PM_GNOME, LOW_BOOTS, GNOMISH_BOOTS },
	 { PM_GNOME, HIGH_BOOTS, GNOMISH_BOOTS },
	 { PM_GNOME, LEATHER_ARMOR, GNOMISH_SUIT },
    { PM_VAMPIRE,	POT_FRUIT_JUICE,	POT_BLOOD	      },
    { PM_VAMPIRE,	FOOD_RATION,		POT_VAMPIRE_BLOOD     },
    { PM_GASTLY,	FOOD_RATION,		CORPSE     },
    { NON_PM,	STRANGE_OBJECT,		STRANGE_OBJECT	      }
};

/* align-based substitutions for initial inventory */
struct inv_asub { aligntyp align; short item_otyp, subs_otyp; };
static struct inv_asub inv_asubs[] = {
    { A_CHAOTIC,	HOLY_WAFER,		LEMBAS_WAFER	      },
    { A_NONE,		STRANGE_OBJECT,		STRANGE_OBJECT	      }
};

/* KMH -- Expectations for skills.
 *	1.	Starting equipment will start as basic, and should have a maximum
 *		of at least skilled.  If you enter the dungeon with it, you should
 *		know how to use it.  See weapon.c, skill_init().
 *	2.	First and second artifact gifts should have a maximum of at least
 *		skilled.  See artifact.c, init_artifacts().
 *	3.	Quest artifacts should have a maximum of expert.  See artifact.c,
 *		init_artifacts().
 *	4.	Races that construct weapons (elf, dwarf, orc) should have
 *		a maximum of expert in those weapons.
 *	5.	Spellcasters should get a maximum of at least basic in all spell
 *		categories.
 */

static const struct def_skill Skill_A[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_PICK_AXE, P_GRAND_MASTER },		{ P_SHORT_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_MASTER },
#endif
    { P_SLING, P_EXPERT },		{ P_DART, P_EXPERT },
    { P_BOOMERANG, P_GRAND_MASTER },		{ P_WHIP, P_MASTER },
    { P_UNICORN_HORN, P_EXPERT },
    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_MASTER },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    /*WAC- 'A' knows of all spells from studying,  but only really good
    at divination class*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Art[] = {

	{ P_KNIFE, P_MASTER },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },    { P_HAMMER, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },    { P_BOW, P_EXPERT },
    { P_DART, P_EXPERT },    { P_DAGGER, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },

	{ P_FIREARM, P_MASTER },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Gam[] = {

    { P_LONG_SWORD, P_EXPERT },		{ P_SHORT_SWORD,  P_EXPERT },

    { P_FLAIL, P_MASTER },		{ P_CLUB,  P_EXPERT },

    { P_KNIFE, P_EXPERT },		{ P_DAGGER,  P_EXPERT },

	{ P_SHURIKEN, P_EXPERT },
	{ P_FIREARM, P_MASTER },
	{ P_BOW, P_MASTER },
	{ P_RIDING, P_MASTER },
	{ P_TWO_WEAPON_COMBAT, P_MASTER },

    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_MASTER },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
	{ P_MATTER_SPELL, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Scr[] = {

    { P_DIVINATION_SPELL, P_EXPERT },
    { P_DAGGER, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_FIREARM, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Sup[] = {

    { P_KNIFE, P_EXPERT },		{ P_UNICORN_HORN,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },		{ P_DART,  P_EXPERT },
    { P_PICK_AXE, P_EXPERT },		{ P_LONG_SWORD,  P_EXPERT },
    { P_ATTACK_SPELL, P_EXPERT },	{ P_DIVINATION_SPELL, P_EXPERT },

    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_FIREARM, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Lib[] = {

    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_MASTER },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },

    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Aug[] = {
    { P_DAGGER, P_EXPERT },		{ P_QUARTERSTAFF,  P_EXPERT },
	{ P_WHIP, P_EXPERT },
	{ P_DIVINATION_SPELL, P_EXPERT },
	{ P_ENCHANTMENT_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
	{ P_MATTER_SPELL, P_EXPERT },

    { P_BARE_HANDED_COMBAT, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Coo[] = {

    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_GRAND_MASTER },
    { P_SLING, P_EXPERT },    { P_CLUB, P_EXPERT },
	{ P_FIREARM, P_EXPERT },	{ P_BOOMERANG, P_MASTER },

	{ P_PROTECTION_SPELL, P_EXPERT },

    { P_BARE_HANDED_COMBAT, P_MASTER },

    { P_NONE, 0 }
};

static const struct def_skill Skill_B[] = {
    { P_DAGGER, P_EXPERT },		{ P_AXE, P_MASTER },
    { P_PICK_AXE, P_EXPERT },	{ P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_EXPERT },	{ P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_MASTER },	{ P_SCIMITAR, P_EXPERT },
    { P_SABER, P_EXPERT },		{ P_CLUB, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },		{ P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },		{ P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_BOW, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            
    { P_ATTACK_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
/*WAC - 'B' is good at attack spells and body manipulation spells
        -both combat oriented */
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_MASTER },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Bard[] = {
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },		{ P_SCIMITAR, P_EXPERT },
    { P_CLUB, P_EXPERT },		{ P_FLAIL, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_POLEARMS, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_SPEAR, P_EXPERT },
    { P_SLING, P_EXPERT },		{ P_DART, P_EXPERT },
    { P_UNICORN_HORN, P_EXPERT },	{ P_CROSSBOW, P_EXPERT },
    { P_ENCHANTMENT_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_C[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },		{ P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_GRAND_MASTER },               { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_TRIDENT, P_EXPERT },
    { P_BOW, P_EXPERT },                { P_SLING, P_MASTER },
    { P_BOOMERANG, P_EXPERT },		{ P_UNICORN_HORN, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },      { P_ENCHANTMENT_SPELL, P_EXPERT },

/*WAC - 'C' is at one with the elements - matter spells, as well as
        basic enchantments - removed attack spell basic skill as
        it is now "dark" spells*/
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ass[] = {

    { P_DAGGER, P_MASTER },             { P_KNIFE,  P_GRAND_MASTER },

    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },
    { P_FIREARM, P_EXPERT },
    { P_BOW, P_EXPERT },    { P_DART, P_EXPERT },

	{ P_DIVINATION_SPELL, P_EXPERT },	{ P_MATTER_SPELL, P_EXPERT },
	{ P_BODY_SPELL, P_EXPERT },

    { P_MARTIAL_ARTS, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Bul[] = {
    { P_CLUB, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_SABER, P_EXPERT },             { P_LONG_SWORD,  P_EXPERT },
    { P_SLING, P_EXPERT },             { P_DART,  P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },             { P_BARE_HANDED_COMBAT,  P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Pic[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_BOW, P_EXPERT },		{ P_DART,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },		{ P_BARE_HANDED_COMBAT,  P_EXPERT },
	{ P_FIREARM,  P_EXPERT },		{ P_TWO_WEAPON_COMBAT,  P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Roc[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },		{ P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },               { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },    { P_FIREARM, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_TRIDENT, P_EXPERT },
    { P_BOW, P_EXPERT },                { P_SLING, P_GRAND_MASTER },
    { P_BOOMERANG, P_EXPERT },		{ P_UNICORN_HORN, P_EXPERT },

    { P_MATTER_SPELL, P_EXPERT },      { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_ATTACK_SPELL, P_EXPERT },      { P_DIVINATION_SPELL, P_EXPERT },

    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_RIDING, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_F[] = {
/*Style: small-med edged weapons, blunt weapons*/
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },
/* Relies on spells for ranged attack*/
    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_MASTER },
/*  Added expert matter spell (elements), skilled in attack, basic in rest
        He is a mage,  so knows the types.*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Fir[] = {
/*Style: small-med edged weapons, blunt weapons*/
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },
/* Relies on spells for ranged attack*/
    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*  Added expert matter spell (elements), skilled in attack, basic in rest
        He is a mage,  so knows the types.*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};


static const struct def_skill Skill_Zyb[] = {

    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },

    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
	{ P_FIREARM, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },
	{ P_LIGHTSABER, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Tha[] = {

    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_MASTER },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },

    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_MASTER },                 { P_SLING, P_EXPERT },
	{ P_FIREARM, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },
	{ P_LIGHTSABER, P_EXPERT },

    { P_ATTACK_SPELL, P_MASTER },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Lun[] = {

    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },

    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
	{ P_FIREARM, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },
	{ P_LIGHTSABER, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_MASTER },
    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_GRAND_MASTER },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Pol[] = {
	{ P_MARTIAL_ARTS, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Bin[] = {

    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },

    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
	{ P_FIREARM, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },
	{ P_LIGHTSABER, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Jes[] = {

    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_WHIP, P_EXPERT },             { P_MATTER_SPELL,  P_EXPERT },
    { P_ATTACK_SPELL, P_EXPERT },             { P_BODY_SPELL,  P_EXPERT },
    { P_CLUB, P_EXPERT },             { P_RIDING,  P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },             { P_ENCHANTMENT_SPELL,  P_EXPERT },
    { P_PICK_AXE, P_EXPERT },             { P_MARTIAL_ARTS,  P_EXPERT },
    { P_FIREARM, P_EXPERT },     { P_BOOMERANG, P_EXPERT }, 

    { P_NONE, 0 }
};

static const struct def_skill Skill_Lad[] = {

	 { P_MARTIAL_ARTS, P_EXPERT },
	 { P_DART, P_EXPERT },
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Blo[] = {

    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_MARTIAL_ARTS, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },               { P_ATTACK_SPELL, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },               { P_LANCE, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },               { P_MATTER_SPELL, P_EXPERT },
    { P_SPEAR, P_EXPERT },               { P_FIREARM, P_EXPERT },
	{ P_LIGHTSABER, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Mys[] = {

    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },

    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
	{ P_FIREARM, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },
	{ P_LIGHTSABER, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ble[] = {

	/* Mainly uses edged weapons. --Amy */

    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_MASTER },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },

    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
	{ P_FIREARM, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },
	{ P_LIGHTSABER, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_GRAND_MASTER },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_MASTER },
    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_MARTIAL_ARTS, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Spa[] = {

    { P_DAGGER, P_MASTER },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },

    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },

	{ P_FIREARM, P_MASTER },

    { P_WHIP, P_EXPERT },                { P_LIGHTSABER, P_MASTER },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ele[] = {
/*Style: small-med edged weapons, blunt weapons*/
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },
/* Relies on spells for ranged attack*/
    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*  Added expert matter spell (elements), skilled in attack, basic in rest
        He is a mage,  so knows the types.*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Aci[] = {
/*Style: small-med edged weapons, blunt weapons*/
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },        { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },        { P_POLEARMS, P_EXPERT },
/* Relies on spells for ranged attack*/
    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_MASTER },
/*  Added expert matter spell (elements), skilled in attack, basic in rest
        He is a mage,  so knows the types.*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

#ifdef CONVICT
static const struct def_skill Skill_Con[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },		    { P_MACE, P_EXPERT },
    { P_DART, P_EXPERT },		    { P_FLAIL, P_GRAND_MASTER },
    { P_SHORT_SWORD, P_EXPERT },		{ P_SLING, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_ATTACK_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};
#endif  /* CONVICT */

static const struct def_skill Skill_Sla[] = {

    { P_SABER, P_EXPERT },		{ P_UNICORN_HORN,  P_EXPERT },
    { P_FIREARM, P_EXPERT },
    { P_WHIP, P_EXPERT },		{ P_SHORT_SWORD,  P_EXPERT },
    { P_DART, P_EXPERT },		{ P_PICK_AXE,  P_EXPERT },

    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Abu[] = {
    { P_WHIP, P_EXPERT },		{ P_FLAIL,  P_EXPERT },
	{ P_PADDLE,  P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_PROTECTION_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_MASTER },
    { P_RIDING, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Dru[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_WHIP, P_EXPERT },		    { P_CLUB, P_EXPERT },		    
    { P_DART, P_EXPERT },		    { P_FLAIL, P_EXPERT },
    { P_SLING, P_EXPERT },    { P_UNICORN_HORN, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_ATTACK_SPELL, P_EXPERT },	{ P_MATTER_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_RIDING, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Tra[] = {
    { P_HAMMER, P_GRAND_MASTER },		{ P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },		    { P_MACE, P_EXPERT },
    { P_FLAIL, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },		{ P_SLING, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_ATTACK_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },	{ P_PROTECTION_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_RIDING, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Act[] = {
    { P_PADDLE, P_GRAND_MASTER },		{ P_WHIP, P_EXPERT },
    { P_CLUB, P_EXPERT },		    { P_SHURIKEN, P_EXPERT },
    { P_FLAIL, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },	{ P_PROTECTION_SPELL, P_EXPERT },
    { P_HEALING_SPELL, P_EXPERT },	{ P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_MASTER },
    { P_RIDING, P_EXPERT },    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Top[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
    { P_AXE, P_EXPERT },		    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_FLAIL, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_TRIDENT, P_EXPERT },
    { P_LANCE, P_EXPERT },		{ P_SLING, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },		{ P_WHIP, P_EXPERT },
	{ P_LIGHTSABER, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_HEALING_SPELL, P_EXPERT },	{ P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },	{ P_PROTECTION_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },	{ P_MATTER_SPELL, P_EXPERT },
    { P_MARTIAL_ARTS, P_MASTER },
    { P_RIDING, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Cou[] = {
    { P_KNIFE,  P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },		    { P_MACE, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },		    { P_FLAIL, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },		{ P_SLING, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_ATTACK_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Gun[] = {

    { P_DAGGER, P_MASTER },		{ P_KNIFE,  P_MASTER },
    { P_FIREARM, P_GRAND_MASTER },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Gan[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_CROSSBOW, P_EXPERT },		{ P_SHURIKEN,  P_EXPERT },
    { P_DART, P_EXPERT },		    { P_WHIP, P_EXPERT },		    

#ifdef FIREARMS
    { P_FIREARM, P_MASTER },
#endif
    { P_ATTACK_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_NONE, 0 }
};

static const struct def_skill Skill_Sci[] = {
    { P_KNIFE,  P_EXPERT },
    { P_CLUB, P_EXPERT },		{ P_HAMMER,  P_EXPERT },
    { P_PADDLE, P_EXPERT },		    { P_FLAIL, P_EXPERT },		    
    { P_POLEARMS, P_EXPERT },		    { P_SLING, P_EXPERT },		    
    { P_WHIP, P_EXPERT },		    { P_LIGHTSABER, P_EXPERT },		    

#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_MATTER_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
	{ P_DIVINATION_SPELL, P_MASTER },
    { P_MARTIAL_ARTS, P_MASTER },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};


static const struct def_skill Skill_G[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },	{ P_CLUB, P_EXPERT },
    { P_PADDLE, P_EXPERT },	{ P_MACE, P_EXPERT },
    { P_FLAIL, P_EXPERT },	{ P_HAMMER, P_EXPERT },
    { P_POLEARMS, P_EXPERT },	{ P_TRIDENT, P_EXPERT },
    { P_LANCE, P_EXPERT },	{ P_SLING, P_EXPERT },
    { P_FIREARM, P_EXPERT },	{ P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },	{ P_BOOMERANG, P_EXPERT },
    { P_LIGHTSABER, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },        
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Gra[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
		{ P_AXE, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },	{ P_CLUB, P_EXPERT },
    { P_PADDLE, P_EXPERT },	{ P_MACE, P_EXPERT },
	{ P_MORNING_STAR, P_EXPERT },	{ P_QUARTERSTAFF, P_EXPERT },
    { P_FLAIL, P_EXPERT },	{ P_HAMMER, P_EXPERT },
    { P_POLEARMS, P_EXPERT },	{ P_JAVELIN, P_EXPERT },
    { P_LANCE, P_EXPERT },	{ P_SLING, P_EXPERT },
    { P_FIREARM, P_EXPERT },	{ P_CROSSBOW, P_EXPERT },
    { P_DART, P_MASTER },	{ P_SHURIKEN, P_EXPERT },
    { P_LIGHTSABER, P_EXPERT },    { P_WHIP, P_EXPERT },
    { P_BOW, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },        
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_ENCHANTMENT_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_MASTER },
    { P_NONE, 0 }
};


static const struct def_skill Skill_H[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_GRAND_MASTER },
    { P_SHORT_SWORD, P_EXPERT },	{ P_SCIMITAR, P_EXPERT },
    { P_SABER, P_EXPERT },		{ P_CLUB, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_TRIDENT, P_EXPERT },
    { P_SLING, P_EXPERT },		{ P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },		{ P_UNICORN_HORN, P_MASTER },

    { P_ATTACK_SPELL, P_EXPERT },        { P_HEALING_SPELL, P_GRAND_MASTER },
    { P_DIVINATION_SPELL, P_EXPERT },    { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};


static const struct def_skill Skill_I[] = {
/*Resorts mostly to stabbing weapons*/
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
  { P_AXE, P_EXPERT },                 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },      { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },          { P_SABER, P_EXPERT },
    { P_MACE, P_EXPERT },                { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },      { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_LANCE, P_EXPERT },
  { P_BOW, P_EXPERT },                 { P_SLING, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },

    { P_ATTACK_SPELL, P_MASTER },        { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },    { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },  { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    /*WAC - same as Flame Mage*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

#ifdef JEDI
static const struct def_skill Skill_J[] = {
    { P_LIGHTSABER, P_GRAND_MASTER },
    { P_SHORT_SWORD, P_EXPERT }, { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT }, { P_SABER, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },{ P_BOOMERANG, P_EXPERT },

#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_HEALING_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_MASTER },    { P_MATTER_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};
#endif

static const struct def_skill Skill_Fox[] = {

    { P_LONG_SWORD, P_EXPERT },                { P_SHORT_SWORD, P_EXPERT },
    { P_DART, P_EXPERT },

    { P_CROSSBOW, P_MASTER },                { P_KNIFE, P_MASTER },
    { P_DAGGER, P_MASTER },                { P_MARTIAL_ARTS, P_MASTER },
    { P_BOW, P_MASTER },                { P_FIREARM, P_MASTER },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Stu[] = {

	{ P_FLAIL, P_GRAND_MASTER },
    { P_WHIP, P_EXPERT },		{ P_HAMMER, P_EXPERT },
    { P_DAGGER, P_EXPERT },		{ P_AXE, P_EXPERT },
    { P_HEALING_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },

    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_RIDING, P_GRAND_MASTER },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Pal[] = {

    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_MASTER },	{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_AXE, P_EXPERT },	

    { P_BOW, P_EXPERT },                { P_CROSSBOW, P_EXPERT },

    { P_CLUB, P_EXPERT },                { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		    { P_QUARTERSTAFF, P_MASTER },		

    { P_PROTECTION_SPELL, P_MASTER },      { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },      { P_HEALING_SPELL, P_MASTER },
	{ P_DIVINATION_SPELL, P_EXPERT },

	{ P_BARE_HANDED_COMBAT, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Wan[] = {

    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },		{ P_FLAIL, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_UNICORN_HORN, P_EXPERT },
	{ P_FIREARM, P_EXPERT },

    { P_PROTECTION_SPELL, P_EXPERT },      { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },      { P_ATTACK_SPELL, P_EXPERT },
    { P_HEALING_SPELL, P_EXPERT },      { P_ENCHANTMENT_SPELL, P_EXPERT },

    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_K[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
    { P_AXE, P_EXPERT },		{ P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },                { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_LANCE, P_GRAND_MASTER },
    { P_BOW, P_EXPERT },			{ P_CROSSBOW, P_EXPERT },
    { P_HEALING_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },      { P_BODY_SPELL, P_EXPERT },
/*WAC - removed the attack spells - no good having knights summoning
        undead!  Replaced with skilled body spells*/

#ifdef STEED
    { P_RIDING, P_MASTER },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Mah[] = {

    { P_ATTACK_SPELL, P_MASTER },      { P_HEALING_SPELL, P_MASTER },
	{ P_BARE_HANDED_COMBAT, P_MASTER },	{ P_FIREARM, P_MASTER },

    { P_PROTECTION_SPELL, P_EXPERT },      { P_DIVINATION_SPELL, P_EXPERT },
	{ P_WHIP, P_EXPERT },		{ P_CLUB, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_EXPERT },
	{ P_BOW, P_EXPERT },		{ P_BOOMERANG, P_EXPERT },
	{ P_QUARTERSTAFF, P_EXPERT },		{ P_AXE, P_EXPERT },
	{ P_RIDING, P_EXPERT },		{ P_CLUB, P_EXPERT },
	{ P_SPEAR, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_Dol[] = {

    { P_KNIFE, P_GRAND_MASTER },
    { P_WHIP, P_MASTER },

    { P_FIREARM, P_GRAND_MASTER },

    { P_DAGGER, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_EXPERT },

    { P_PROTECTION_SPELL, P_EXPERT },      { P_MATTER_SPELL, P_EXPERT },
    { P_ATTACK_SPELL, P_EXPERT },      { P_DIVINATION_SPELL, P_EXPERT },
	{ P_ENCHANTMENT_SPELL, P_EXPERT },

    { P_NONE, 0 }
};

static const struct def_skill Skill_War[] = { /*master of all weapons*/
    { P_DAGGER, P_MASTER },		{ P_KNIFE, P_MASTER },
    { P_AXE, P_MASTER },		{ P_PICK_AXE, P_MASTER },		
    { P_SHORT_SWORD, P_MASTER },	{ P_BROAD_SWORD, P_MASTER },
    { P_LONG_SWORD, P_MASTER },	{ P_TWO_HANDED_SWORD, P_MASTER },
    { P_SCIMITAR, P_MASTER },		{ P_SABER, P_MASTER },
    { P_CLUB, P_MASTER },                { P_PADDLE, P_MASTER },
    { P_MACE, P_MASTER },
    { P_MORNING_STAR, P_MASTER },	{ P_FLAIL, P_MASTER },
    { P_HAMMER, P_MASTER },		{ P_QUARTERSTAFF, P_MASTER },		{ P_POLEARMS, P_MASTER },
    { P_SPEAR, P_MASTER },		{ P_JAVELIN, P_MASTER },
    { P_TRIDENT, P_MASTER },		{ P_LANCE, P_MASTER },
    { P_BOW, P_MASTER },			{ P_CROSSBOW, P_MASTER },
    { P_SLING, P_MASTER },			{ P_DART, P_MASTER },
#ifdef FIREARMS
    { P_FIREARM, P_MASTER },
#endif
    { P_SHURIKEN, P_MASTER },			{ P_BOOMERANG, P_MASTER },
    { P_WHIP, P_MASTER },			{ P_UNICORN_HORN, P_MASTER },
    { P_LIGHTSABER, P_MASTER },
/* no magic skills at all*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ord[] = { /*master of all weapons*/
    { P_DAGGER, P_GRAND_MASTER },		{ P_KNIFE, P_GRAND_MASTER},
    { P_AXE, P_GRAND_MASTER},		{ P_PICK_AXE, P_GRAND_MASTER},		
    { P_SHORT_SWORD, P_GRAND_MASTER},	{ P_BROAD_SWORD, P_GRAND_MASTER},
    { P_LONG_SWORD, P_GRAND_MASTER},	{ P_TWO_HANDED_SWORD, P_GRAND_MASTER},
    { P_SCIMITAR, P_GRAND_MASTER},		{ P_SABER, P_GRAND_MASTER},
    { P_CLUB, P_GRAND_MASTER},                { P_PADDLE, P_GRAND_MASTER},
    { P_MACE, P_GRAND_MASTER},
    { P_MORNING_STAR, P_GRAND_MASTER},	{ P_FLAIL, P_GRAND_MASTER},
    { P_HAMMER, P_GRAND_MASTER},		{ P_QUARTERSTAFF, P_GRAND_MASTER},		{ P_POLEARMS, P_GRAND_MASTER},
    { P_SPEAR, P_GRAND_MASTER},		{ P_JAVELIN, P_GRAND_MASTER},
    { P_TRIDENT, P_GRAND_MASTER},		{ P_LANCE, P_GRAND_MASTER},
    { P_BOW, P_GRAND_MASTER},			{ P_CROSSBOW, P_GRAND_MASTER},
    { P_SLING, P_GRAND_MASTER},			{ P_DART, P_GRAND_MASTER},
#ifdef FIREARMS
    { P_FIREARM, P_GRAND_MASTER},
#endif
    { P_SHURIKEN, P_GRAND_MASTER},			{ P_BOOMERANG, P_GRAND_MASTER},
    { P_WHIP, P_GRAND_MASTER},			{ P_UNICORN_HORN, P_GRAND_MASTER},
    { P_LIGHTSABER, P_GRAND_MASTER},
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
	{ P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Off[] = { /*master of all weapons*/
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
    { P_AXE, P_EXPERT },		{ P_PICK_AXE, P_EXPERT },		
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },                { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },		{ P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },			{ P_CROSSBOW, P_EXPERT },
    { P_SLING, P_EXPERT },			{ P_DART, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_SHURIKEN, P_EXPERT },			{ P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },			{ P_UNICORN_HORN, P_EXPERT },
    { P_LIGHTSABER, P_EXPERT },
/* no magic skills at all*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Mon[] = {
    { P_PADDLE, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_BOW, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_DART, P_EXPERT },    { P_CROSSBOW, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },    { P_UNICORN_HORN, P_EXPERT },


    { P_ATTACK_SPELL, P_EXPERT },    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },	{ P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_MASTER },	{ P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*WAC - monks are good healers - expert healing - and expert protect*/
    { P_NONE, 0 }
};

static const struct def_skill Skill_Sai[] = {
    { P_PADDLE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_BOW, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_DART, P_EXPERT },    { P_CROSSBOW, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },    { P_UNICORN_HORN, P_EXPERT },


    { P_ATTACK_SPELL, P_EXPERT },    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },	{ P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_MASTER },	{ P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Psi[] = {
    { P_PADDLE, P_EXPERT },{ P_FIREARM, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_BOW, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },            { P_MARTIAL_ARTS, P_GRAND_MASTER },

    { P_ATTACK_SPELL, P_EXPERT },    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_MASTER },	{ P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_GRAND_MASTER },
    { P_MATTER_SPELL, P_MASTER },
/*WAC - monks are good healers - expert healing - and expert protect*/
    { P_NONE, 0 }
};

static const struct def_skill Skill_Nob[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
    { P_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },		{ P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },			{ P_CROSSBOW, P_EXPERT },
	{ P_FIREARM, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },	{ P_ENCHANTMENT_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_N[] = {
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },               { P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_MACE, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },       { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },               { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },             { P_SLING, P_EXPERT },
    { P_DART, P_EXPERT },               { P_SHURIKEN, P_EXPERT },
    { P_CROSSBOW, P_GRAND_MASTER },
    { P_FLAIL, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif

    { P_ATTACK_SPELL, P_MASTER },
    { P_MATTER_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_ENCHANTMENT_SPELL, P_EXPERT },
/*WAC-  expert of dark arts - attack spells,  skilled in matter
        -for fireball and cone of cold*/
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static struct def_skill Skill_P[] = {
	/* KMH -- Long sword for Sunsword */
	{ P_LONG_SWORD, P_EXPERT },
    { P_CLUB, P_EXPERT },               { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_TRIDENT, P_EXPERT },
    { P_LANCE, P_EXPERT },		{ P_BOW, P_EXPERT },
    { P_SLING, P_EXPERT },		{ P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },		{ P_UNICORN_HORN, P_MASTER },

    /* [ALI] Depending on the spellbook which priests enter the dungeon with,
     * one of the maximum skill levels listed here will be raised by one.
     */
    { P_ATTACK_SPELL, P_EXPERT },        { P_HEALING_SPELL, P_MASTER },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },  { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

    { P_BARE_HANDED_COMBAT, P_EXPERT },  /* the monk is added in slash */ 
    { P_NONE, 0 }
};

static struct def_skill Skill_Che[] = {

	{ P_LONG_SWORD, P_EXPERT },
    { P_CLUB, P_EXPERT },               { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_TRIDENT, P_EXPERT },
    { P_LANCE, P_EXPERT },		{ P_BOW, P_EXPERT },
    { P_SLING, P_EXPERT },		{ P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },		{ P_UNICORN_HORN, P_EXPERT },

    /* [ALI] Depending on the spellbook which priests enter the dungeon with,
     * one of the maximum skill levels listed here will be raised by one.
     */
    { P_ATTACK_SPELL, P_EXPERT },        { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_MASTER },
    { P_PROTECTION_SPELL, P_EXPERT },  { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

    { P_BARE_HANDED_COMBAT, P_EXPERT },  /* the monk is added in slash */ 
    { P_RIDING, P_EXPERT },
    { P_NONE, 0 }
};


static const struct def_skill Skill_Pir[] = {
    { P_DAGGER, P_EXPERT },	{ P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },	    { P_SHORT_SWORD, P_EXPERT },
	{ P_BROAD_SWORD, P_EXPERT },{ P_LONG_SWORD, P_EXPERT },
	{ P_SCIMITAR, P_GRAND_MASTER },	{ P_SABER, P_EXPERT },
	{ P_CLUB, P_EXPERT },		{ P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },		{ P_SPEAR, P_EXPERT },
	{ P_JAVELIN, P_EXPERT },	{ P_TRIDENT, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },   { P_DART, P_EXPERT },
    { P_WHIP, P_EXPERT },   	{ P_UNICORN_HORN, P_EXPERT },
	{ P_FIREARM, P_MASTER },
	{ P_ATTACK_SPELL, P_EXPERT },{ P_DIVINATION_SPELL, P_EXPERT },
	{ P_ENCHANTMENT_SPELL, P_EXPERT },{ P_BODY_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Div[] = {
    { P_TRIDENT, P_GRAND_MASTER },	{ P_KNIFE,  P_EXPERT },
    { P_FLAIL, P_EXPERT },	    { P_SHORT_SWORD, P_EXPERT },
	{ P_DAGGER, P_EXPERT },{ P_SPEAR, P_EXPERT },
	{ P_JAVELIN, P_EXPERT },	{ P_FIREARM, P_EXPERT },

	{ P_DIVINATION_SPELL, P_EXPERT }, { P_BODY_SPELL, P_MASTER },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_RIDING, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Kor[] = {
    { P_DAGGER, P_EXPERT },	{ P_LONG_SWORD,  P_EXPERT },
    { P_PICK_AXE, P_EXPERT },	    { P_SHORT_SWORD, P_EXPERT },
	{ P_DART, P_EXPERT },{ P_KNIFE, P_EXPERT },
	{ P_AXE, P_EXPERT },	{ P_POLEARMS, P_EXPERT },
	{ P_SPEAR, P_EXPERT },		{ P_TRIDENT, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_WHIP, P_EXPERT },
	{ P_SABER, P_EXPERT },	{ P_FIREARM, P_GRAND_MASTER },

	{ P_ATTACK_SPELL, P_EXPERT },{ P_DIVINATION_SPELL, P_EXPERT },

    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Gla[] = {
    { P_SABER, P_EXPERT },	{ P_PICK_AXE,  P_EXPERT },
    { P_UNICORN_HORN, P_EXPERT },	    { P_CROSSBOW, P_EXPERT },
	{ P_KNIFE, P_EXPERT },{ P_JAVELIN, P_EXPERT },
	{ P_POLEARMS, P_EXPERT },	{ P_SCIMITAR, P_EXPERT },
	{ P_FLAIL, P_EXPERT },		{ P_BROAD_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },		{ P_MORNING_STAR, P_EXPERT },
	{ P_HAMMER, P_EXPERT },	{ P_LANCE, P_EXPERT },
    { P_WHIP, P_EXPERT },   { P_LONG_SWORD, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },   	{ P_DAGGER, P_EXPERT },
	{ P_SPEAR, P_EXPERT },	{ P_TRIDENT, P_EXPERT },

	{ P_ATTACK_SPELL, P_EXPERT },{ P_BODY_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_RIDING, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Gof[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },    { P_FIREARM, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },  { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },  { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif

    { P_NONE, 0 }
};

static const struct def_skill Skill_R[] = {
    { P_DAGGER, P_MASTER },		{ P_KNIFE,  P_GRAND_MASTER },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },              { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_POLEARMS, P_EXPERT },            { P_SPEAR, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_MASTER },		{ P_SHURIKEN, P_MASTER },
    { P_DIVINATION_SPELL, P_EXPERT },        { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*WAC Left as is*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Dun[] = {
    { P_DAGGER, P_GRAND_MASTER },		{ P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },              { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_POLEARMS, P_EXPERT },            { P_SPEAR, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_BOW, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },        { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*WAC Left as is*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Loc[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },              { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_POLEARMS, P_EXPERT },            { P_SPEAR, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },        { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*WAC Left as is*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};


static const struct def_skill Skill_Nin[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },              { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },               { P_HAMMER, P_EXPERT },
    { P_POLEARMS, P_EXPERT },            { P_SPEAR, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_GRAND_MASTER },
    { P_DIVINATION_SPELL, P_EXPERT },        { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    { P_HEALING_SPELL, P_EXPERT },    { P_PROTECTION_SPELL, P_EXPERT },

/*WAC Left as is*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ran[] = {
    { P_DAGGER, P_EXPERT },		 { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },	 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	 { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },	 { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT }, { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },	 { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },	 { P_BOW, P_MASTER },
    { P_SLING, P_EXPERT },	 { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },	 { P_SHURIKEN, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },	 { P_WHIP, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Elp[] = {
    { P_DAGGER, P_EXPERT },		 { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },	 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	 { P_MORNING_STAR, P_EXPERT },

    { P_BROAD_SWORD, P_EXPERT },	 { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },	 { P_SABER, P_EXPERT },

    { P_FLAIL, P_EXPERT },	 { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT }, { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },	 { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },	 { P_BOW, P_GRAND_MASTER },
    { P_SLING, P_EXPERT },	 { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },	 { P_SHURIKEN, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },	 { P_WHIP, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Bos[] = {
    { P_DAGGER, P_EXPERT },		 { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },	 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	 { P_MORNING_STAR, P_EXPERT },

    { P_BROAD_SWORD, P_EXPERT },	 { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },	 { P_SABER, P_EXPERT },

    { P_FLAIL, P_EXPERT },	 { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT }, { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },	 { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },	 { P_BOW, P_GRAND_MASTER },
    { P_SLING, P_EXPERT },	 { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },	 { P_SHURIKEN, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },	 { P_WHIP, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Mus[] = {
    { P_DAGGER, P_EXPERT },		 { P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },	 { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	 { P_MORNING_STAR, P_EXPERT },

    { P_BROAD_SWORD, P_EXPERT },	 { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },	 { P_SABER, P_EXPERT },

    { P_FLAIL, P_EXPERT },	 { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT }, { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },	 { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },	 { P_BOW, P_EXPERT },
    { P_SLING, P_EXPERT },	 { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },	 { P_SHURIKEN, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },	 { P_WHIP, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_UNICORN_HORN, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_S[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_GRAND_MASTER },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },		{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_FLAIL, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },		{ P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },      { P_BODY_SPELL, P_EXPERT },

/* WAC - removed the attack spells, replace with body.  clairvoyance, meditate*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ama[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
	{ P_AXE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },		{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_PADDLE, P_EXPERT },    { P_CLUB, P_EXPERT },
    { P_MACE, P_EXPERT },    { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },		{ P_HAMMER, P_EXPERT },		
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_MASTER },
    { P_JAVELIN, P_GRAND_MASTER },		{ P_LANCE, P_MASTER },{ P_TRIDENT, P_MASTER },
    { P_BOW, P_MASTER },		{ P_SLING, P_EXPERT },{ P_FIREARM, P_MASTER },
	{ P_CROSSBOW, P_EXPERT },{ P_DART, P_EXPERT },
	{ P_WHIP,  P_EXPERT },

    { P_PROTECTION_SPELL, P_EXPERT },      { P_BODY_SPELL, P_EXPERT },{ P_ATTACK_SPELL, P_EXPERT },

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ota[] = {

	{ P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_SCIMITAR, P_EXPERT },
    { P_BROAD_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SABER, P_EXPERT },	{ P_MACE, P_EXPERT },
    { P_FLAIL, P_EXPERT },	{ P_AXE, P_EXPERT },
    { P_PICK_AXE, P_EXPERT },	{ P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },	{ P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },	{ P_BOW, P_EXPERT },
    { P_WHIP, P_EXPERT },	{ P_CROSSBOW, P_EXPERT },
    { P_DAGGER, P_EXPERT },	{ P_SLING, P_EXPERT },
    { P_DART, P_EXPERT },	{ P_HAMMER, P_EXPERT },
    { P_UNICORN_HORN, P_EXPERT },	{ P_LONG_SWORD, P_MASTER },
    { P_QUARTERSTAFF, P_MASTER },	{ P_SHURIKEN, P_MASTER },

    { P_DIVINATION_SPELL, P_EXPERT },	{ P_MATTER_SPELL, P_EXPERT },
	{ P_BODY_SPELL, P_EXPERT },	{ P_FIREARM, P_EXPERT },
	{ P_LIGHTSABER, P_MASTER },
    { P_NONE, 0 }
};

#ifdef TOURIST
static const struct def_skill Skill_T[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },			{ P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },		{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },		{ P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },		{ P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_LANCE, P_EXPERT },
    { P_BOW, P_EXPERT },			{ P_SLING, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_EXPERT },		{ P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },		{ P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },		{ P_UNICORN_HORN, P_EXPERT },
    { P_DIVINATION_SPELL, P_GRAND_MASTER },	{ P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },
/*WAC left alone*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};
#endif /* TOURIST */

static const struct def_skill Skill_U[] = {
/*WAC
 * -made dagger skill expert too,  since it's a starting weapon
 * -made spear skill Expert rather than Skilled
 *      Slayer artifact is a spear,  after all
 * -made crossbow skill Expert - Dracula movies
 * -changed bare handed basic to martial arts master-Buffy the Vampire Slayer
 * -Added whip Expert - Castlevania
 * -made club, flail, mace, morning star, hammer, quarterstaff Skilled
        from Expert to balance
 * -removed Trident skill - from Skilled to Restricted
 * -removed Lance skill - from Basic to Restricted
 */
    { P_DAGGER, P_EXPERT },             { P_LONG_SWORD, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },              { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },             { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },      { P_POLEARMS, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },	{ P_SCIMITAR, P_EXPERT },
    { P_SPEAR, P_EXPERT },              { P_JAVELIN, P_EXPERT },
    { P_BOW, P_EXPERT },			        { P_SLING, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_MASTER },           { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },		{ P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_MASTER },               { P_UNICORN_HORN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },      { P_LANCE, P_EXPERT },

    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },    { P_ATTACK_SPELL, P_EXPERT },
/*WAC - added PROTECTION spells,  body spells as skilled, basic
        matter spells - for the fire vs undead*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_GRAND_MASTER },    { P_NONE, 0 }
};

static const struct def_skill Skill_Unt[] = {
    { P_DAGGER, P_EXPERT },             { P_LONG_SWORD, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },              { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },             { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },      { P_POLEARMS, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },	{ P_SCIMITAR, P_EXPERT },
    { P_SPEAR, P_EXPERT },              { P_JAVELIN, P_EXPERT },
    { P_BOW, P_EXPERT },			        { P_SLING, P_EXPERT },

    { P_PICK_AXE, P_MASTER }, /* thanks 5lo for reminding me that undertakers are supposed to have that skill --Amy */

#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_EXPERT },           { P_DART, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },		{ P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },               { P_UNICORN_HORN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },      { P_LANCE, P_EXPERT },

    { P_PROTECTION_SPELL, P_EXPERT },    { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_GRAND_MASTER },    { P_ATTACK_SPELL, P_EXPERT },
    { P_ENCHANTMENT_SPELL, P_EXPERT },    { P_DIVINATION_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_GRAND_MASTER },    { P_NONE, 0 }
};

static const struct def_skill Skill_V[] = {
    { P_DAGGER, P_EXPERT },		{ P_AXE, P_EXPERT },
    { P_PICK_AXE, P_EXPERT },		{ P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_EXPERT },	{ P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },	{ P_SCIMITAR, P_EXPERT },
    { P_SABER, P_EXPERT },		{ P_HAMMER, P_MASTER },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_LANCE, P_EXPERT },
    { P_SLING, P_EXPERT },
    { P_BOW, P_EXPERT },    { P_CROSSBOW, P_EXPERT },

    { P_BODY_SPELL, P_EXPERT },          { P_MATTER_SPELL, P_EXPERT },
/*  replace attack spell with matter spell - cone of cold, lightning
 *  Boosted to Expert
 */
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Sag[] = {

    { P_DAGGER, P_EXPERT },		{ P_CLUB,  P_EXPERT },
    { P_DART, P_EXPERT },		{ P_CROSSBOW, P_EXPERT },

    { P_QUARTERSTAFF, P_EXPERT },		{ P_HAMMER,  P_EXPERT },
    { P_UNICORN_HORN, P_EXPERT },		{ P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },		{ P_FLAIL,  P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },  { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_PROTECTION_SPELL, P_EXPERT },  { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },

    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_W[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },		{ P_SHORT_SWORD, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_QUARTERSTAFF, P_MASTER },	{ P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_SLING, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif

    { P_ATTACK_SPELL, P_GRAND_MASTER },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },  { P_ENCHANTMENT_SPELL, P_MASTER },
    { P_PROTECTION_SPELL, P_MASTER },  { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*      added matter spell skilled,  as fireball and cone of cold are
        matter spells, but now specialty of F/I Mages*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Alt[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_AXE, P_EXPERT },		{ P_SHORT_SWORD, P_EXPERT },
    { P_CLUB, P_EXPERT },              { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_SLING, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif

    { P_ATTACK_SPELL, P_GRAND_MASTER },	{ P_HEALING_SPELL, P_GRAND_MASTER},
    { P_DIVINATION_SPELL, P_GRAND_MASTER},  { P_ENCHANTMENT_SPELL, P_GRAND_MASTER},
    { P_PROTECTION_SPELL, P_GRAND_MASTER},  { P_BODY_SPELL, P_GRAND_MASTER},
    { P_MATTER_SPELL, P_GRAND_MASTER},
/*      added matter spell skilled,  as fireball and cone of cold are
        matter spells, but now specialty of F/I Mages*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Pok[] = {

    { P_ATTACK_SPELL, P_MASTER },	{ P_HEALING_SPELL, P_MASTER },
    { P_DIVINATION_SPELL, P_MASTER },  { P_ENCHANTMENT_SPELL, P_MASTER },
    { P_PROTECTION_SPELL, P_MASTER },  { P_BODY_SPELL, P_MASTER },
    { P_MATTER_SPELL, P_MASTER },
/*      added matter spell skilled,  as fireball and cone of cold are
        matter spells, but now specialty of F/I Mages*/
#ifdef STEED
    { P_RIDING, P_GRAND_MASTER },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Zoo[] = {

    { P_WHIP, P_EXPERT },
    { P_SPEAR, P_EXPERT },    { P_JAVELIN, P_EXPERT },
    { P_POLEARMS, P_EXPERT },    { P_FIREARM, P_EXPERT },
    { P_BOW, P_EXPERT },    { P_CROSSBOW, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },

    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },  
    { P_PROTECTION_SPELL, P_EXPERT },  { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*      added matter spell skilled,  as fireball and cone of cold are
        matter spells, but now specialty of F/I Mages*/
#ifdef STEED
    { P_RIDING, P_GRAND_MASTER },
#endif
    { P_MARTIAL_ARTS, P_MASTER },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Dea[] = {
    { P_KNIFE,  P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },	
    { P_TRIDENT,  P_EXPERT },
    { P_DART,  P_EXPERT },     { P_SHURIKEN,  P_EXPERT },
    { P_WHIP,  P_EXPERT },     { P_UNICORN_HORN,  P_EXPERT },

#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif

    { P_ATTACK_SPELL, P_GRAND_MASTER },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },  { P_ENCHANTMENT_SPELL, P_MASTER },
    { P_PROTECTION_SPELL, P_EXPERT },  { P_BODY_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_MASTER },
/*      added matter spell skilled,  as fireball and cone of cold are
        matter spells, but now specialty of F/I Mages*/
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_MARTIAL_ARTS, P_MASTER },
    { P_NONE, 0 }
};

#ifdef YEOMAN
static const struct def_skill Skill_Y[] = {
    { P_DAGGER, P_EXPERT },            { P_KNIFE, P_EXPERT },
    { P_AXE, P_EXPERT },               { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_EXPERT },         { P_LONG_SWORD, P_EXPERT },
    { P_SABER, P_EXPERT },             { P_PADDLE, P_EXPERT },
    { P_HAMMER, P_EXPERT },              { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_GRAND_MASTER },           { P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_EXPERT },           { P_TRIDENT, P_EXPERT },
    { P_LANCE, P_EXPERT },             { P_BOW, P_EXPERT },
    { P_SLING, P_EXPERT },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_EXPERT },          { P_DART, P_EXPERT },
    { P_WHIP, P_EXPERT },                { P_UNICORN_HORN, P_EXPERT },

    { P_ENCHANTMENT_SPELL, P_EXPERT }, { P_PROTECTION_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_GRAND_MASTER },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT }, { P_MARTIAL_ARTS, P_MASTER },
    { P_NONE, 0 }
};
#endif


STATIC_OVL void
knows_object(obj)
register int obj;
{
	discover_object(obj,TRUE,FALSE);
	objects[obj].oc_pre_discovered = 1;	/* not a "discovery" */
}

/* Know ordinary (non-magical) objects of a certain class,
 * like all gems except the loadstone and luckstone.
 */
STATIC_OVL void
knows_class(sym)
register char sym;
{
	register int ct;
	for (ct = 1; ct < NUM_OBJECTS; ct++)
		if (objects[ct].oc_class == sym && !objects[ct].oc_magic)
			knows_object(ct);
}

/* Know all objects of a certain class, like all potions (healer). */
STATIC_OVL void
knows_classX(sym)
register char sym;
{
	register int ct;
	for (ct = 1; ct < NUM_OBJECTS; ct++)
		if (objects[ct].oc_class == sym)
			knows_object(ct);
}

/* [ALI] Raise one spell skill by one level. Priorities:
 * - The skill for the chosen spellbook if not already expert.
 * - A skill currently at skilled level.
 * - A skill currently at basic level.
 * Where more than one skill is possible at a priority level, choose one
 * at random.
 *
 * The idea is that where a role may be given spellbooks in which the
 * role is normally at basic level, then the Skill array can be tweaked
 * to reduce one skill from expert to skilled. After choosing the
 * spellbook we can then dynamically raise one skill which will either be
 * the one for the spellbook if that is currently basic (and so avoid the
 * warning message from skill_init) or raise the tweaked skill to expert.
 *
 * Currently only used by priests.
 */

static void
spellbook_skill_raise(class_skill, spellbook)
register struct def_skill *class_skill;
int spellbook;
{
    register int i, j;
    j = spell_skilltype(spellbook);
    for(i = 0; class_skill[i].skill != P_NONE; i++)
	if (class_skill[i].skill == j)
	    break;
    if (class_skill[i].skill == P_NONE)
	pline("Warning: No entry for %s in Skill array.",
	  obj_typename(spellbook));
    else if (class_skill[i].skmax < P_EXPERT)
	class_skill[i].skmax++;
    else
    {
	j = 0;
	for(i = 0; class_skill[i].skill != P_NONE; i++) {
	    if (class_skill[i].skill >= P_FIRST_SPELL &&
	      class_skill[i].skill <= P_LAST_SPELL &&
	      class_skill[i].skmax == P_SKILLED)
		j++;
	}
	if (j) {
	    j = rn2(j);
	    for(i = 0; class_skill[i].skill != P_NONE; i++) {
		if (class_skill[i].skill >= P_FIRST_SPELL &&
		  class_skill[i].skill <= P_LAST_SPELL &&
		  class_skill[i].skmax == P_SKILLED)
		    if (!j--) {
			class_skill[i].skmax++;
			break;
		    }
	    }
	}
	else {
	    for(i = 0; class_skill[i].skill != P_NONE; i++) {
		if (class_skill[i].skill >= P_FIRST_SPELL &&
		  class_skill[i].skill <= P_LAST_SPELL &&
		  class_skill[i].skmax >= P_BASIC &&
		  class_skill[i].skmax < P_EXPERT)
		    j++;
	    }
	    if (j) {
		j = rn2(j);
		for(i = 0; class_skill[i].skill != P_NONE; i++) {
		    if (class_skill[i].skill >= P_FIRST_SPELL &&
		      class_skill[i].skill <= P_LAST_SPELL &&
		      class_skill[i].skmax >= P_BASIC &&
		      class_skill[i].skmax < P_EXPERT)
			if (!j--) {
			    class_skill[i].skmax++;
			    break;
			}
		}
	    }
	}
    }
}

/* Know 5 to 15 random magical objects (wands, potions, scrolls, ...)
   For now we decide that tools while possibly magical (bag of holding/tricks,
   magic lamp) are excempt because the Bard already knows all instruments
   which appear to be more than enough tools.
   We might also add GEM_CLASS with oc_material != GLASS 
*** Contributed by Johanna Ploog */
STATIC_OVL void
know_random_obj()
{
        register int obj, count, ct;

        count = rn1(11,5);
        for (ct = 500; ct > 0 && count > 0; ct--) {
           obj = rn2(NUM_OBJECTS);
           if (objects[obj].oc_magic &&

               /* We have to make an exception for those dummy
                  objects (wand and scroll) that exist to allow
                  for additional descriptions. */

               obj_descr[(objects[obj].oc_name_idx)].oc_name != 0 /*&&

              (objects[obj].oc_class == ARMOR_CLASS &&*/

               /* Dragon scales and mails are considered magical,
                  but as they don't have different descriptions,
                  they don't appear in the discovery list,
                  so as not to rob the player of an opportunity... */

                /*!(obj > HELM_OF_TELEPATHY && obj < PLATE_MAIL) ||

               objects[obj].oc_class == RING_CLASS ||
               objects[obj].oc_class == POTION_CLASS ||
               objects[obj].oc_class == SCROLL_CLASS ||
               objects[obj].oc_class == SPBOOK_CLASS ||
               objects[obj].oc_class == WAND_CLASS ||
               objects[obj].oc_class == AMULET_CLASS)*/)
            {
              knows_object(obj);
              count--;
            }
        }
}

void
u_init()
{
	register int i, temp, racebounus, rolebounus, alignbounus, genderbounus, maxbounus;
	struct permonst* shamblerm = &mons[PM_NITROHACK_HORROR];
	struct permonst* shamblerma = &mons[PM_SPEEDHACK_HORROR];
	struct permonst* shamblern = &mons[PM_NETHACKFOUR_HORROR];
	struct permonst* shamblerna = &mons[PM_DEVTEAM_HORROR];
	struct permonst* shamblerl = &mons[PM_DNETHACK_HORROR];
	struct permonst* shamblerla = &mons[PM_NETHACKBRASS_HORROR];
	struct permonst* shamblerx = &mons[PM_INTERHACK_HORROR];
	struct permonst* shamblerxa = &mons[PM_NHTNG_HORROR];
	struct permonst* shambler = &mons[PM_UNNETHACK_HORROR];
	struct permonst* shamblerp = &mons[PM_UNNETHACKPLUS_HORROR];
	struct permonst* shamblera = &mons[PM_ANGBAND_HORROR];
	struct permonst* shamblerap = &mons[PM_ADOM_HORROR];
	struct permonst* shamblerb = &mons[PM_PETTY_ANGBAND_HORROR];
	struct permonst* shamblerbp = &mons[PM_PETTY_ADOM_HORROR];
	struct permonst* shamblers = &mons[PM_SPORKHACK_HORROR];
	struct permonst* shamblersp = &mons[PM_SLASHEM_HORROR];
	struct permonst* shamblerr = &mons[PM_NETHACK_HORROR];
	struct permonst* shamblerrp = &mons[PM_ROGUE_HORROR];	
	struct permonst* shamblert = &mons[PM_GRUNTHACK_HORROR];
	struct permonst* shamblerta = &mons[PM_ACEHACK_HORROR];
	struct permonst* shamblery = &mons[PM_MAIDENHACK_HORROR];
	struct permonst* shamblerya = &mons[PM_YASD_HORROR];
	struct permonst* shamblertp = &mons[PM_PETTY_GRUNTHACK_HORROR];
	struct permonst* shamblertpa = &mons[PM_PETTY_ACEHACK_HORROR];

	struct permonst* shamblerz = &mons[PM_STONE_COLD_HORROR];
	struct permonst* shamblerza = &mons[PM_PETROGRAPHY_HORROR];

	struct permonst* shamblerplayer = &mons[PM_AK_THIEF_IS_DEAD_];
	struct permonst* shamblerplayerb = &mons[PM_UN_IN_PROTECT_MODE];

	struct permonst* shamblerplayerc = &mons[PM_BROWN_MISSINGNO];
	struct permonst* shamblerplayerd = &mons[PM_RED_MISSINGNO];
	struct permonst* shamblerplayere = &mons[PM_BLACK_MISSINGNO];
	struct permonst* shamblerplayerf = &mons[PM_CYAN_MISSINGNO];
	struct permonst* shamblerplayerg = &mons[PM_GRAY_MISSINGNO];
	struct permonst* shamblerplayerh = &mons[PM_WHITE_MISSINGNO];
	struct permonst* shamblerplayeri = &mons[PM_GREEN_MISSINGNO];
	struct permonst* shamblerplayerj = &mons[PM_MAGENTA_MISSINGNO];
	struct permonst* shamblerplayerk = &mons[PM_YELLOW_MISSINGNO];
	struct permonst* shamblerplayerl = &mons[PM_ORANGE_MISSINGNO];
	struct permonst* shamblerplayerm = &mons[PM_BRIGHT_CYAN_MISSINGNO];
	struct permonst* shamblerplayern = &mons[PM_BRIGHT_MAGENTA_MISSINGNO];
	struct permonst* shamblerplayero = &mons[PM_BRIGHT_BLUE_MISSINGNO];
	struct permonst* shamblerplayerp = &mons[PM_BRIGHT_GREEN_MISSINGNO];

	struct permonst* deathraylord = &mons[PM_YEENOGHU];
	struct permonst* multigrue = &mons[PM_MULTICOLOR_GRUE];

	struct permonst* randomeye = &mons[PM_MYSTIC_EYE];
	struct permonst* randommimic = &mons[PM_UNKNOWN_MIMIC];
	struct permonst* randompiercer = &mons[PM_SHINING_PIERCER];
	struct permonst* randompiercerb = &mons[PM_SHINING_PENETRATOR];
	struct permonst* randompiercerc = &mons[PM_SHINING_SMASHER];
	struct permonst* randomvortex = &mons[PM_SIZZLING_VORTEX];
	struct permonst* randomfungus = &mons[PM_COLORLESS_MOLD];
	struct permonst* randomfungusb = &mons[PM_COLORLESS_FUNGUS];
	struct permonst* randomfungusc = &mons[PM_NONDESCRIPT_MOLD];
	struct permonst* randomfungusd = &mons[PM_NONDESCRIPT_FUNGUS];
	struct permonst* randomkop = &mons[PM_ONG_SEPHIRAH];
	struct permonst* randomkopb = &mons[PM_KRONG_SEPHIRAH];
	struct permonst* randomkopc = &mons[PM_ZAKRONG_SEPHIRAH];
	struct permonst* randomtroll = &mons[PM_SPECIAL_TROLL];
	struct permonst* randomeel = &mons[PM_TEETHFISH];
	struct permonst* randomflyfish = &mons[PM_RAZORFIN_FISH];
	struct permonst* randomturret = &mons[PM_UNKNOWN_TURRET];
	struct permonst* randomdragon = &mons[PM_RAINBOW_MODE_DRAGON];

	struct permonst* shadowwarrior = &mons[PM_SHADOW_WARRIOR];
	struct permonst* shadowwarriorX = &mons[PM_ROBOT];

	struct permonst* randomdragonb = &mons[PM_ADULT_TATZELWORM];
	struct permonst* randomdragonc = &mons[PM_ADULT_AMPHITERE];
	struct permonst* randomdragond = &mons[PM_ADULT_DRAKEN];
	struct permonst* randomdragone = &mons[PM_ADULT_LINDWORM];
	struct permonst* randomdragonf = &mons[PM_ADULT_SARKANY];
	struct permonst* randomdragong = &mons[PM_ADULT_SIRRUSH];
	struct permonst* randomdragonh = &mons[PM_ADULT_LEVIATHAN];
	struct permonst* randomdragoni = &mons[PM_ADULT_WYVERN];
	struct permonst* randomdragonj = &mons[PM_ADULT_GLOWING_DRAGON];
	struct permonst* randomdragonk = &mons[PM_ADULT_GUIVRE];

	struct permonst* randombdragonb = &mons[PM_BABY_TATZELWORM];
	struct permonst* randombdragonc = &mons[PM_BABY_AMPHITERE];
	struct permonst* randombdragond = &mons[PM_BABY_DRAKEN];
	struct permonst* randombdragone = &mons[PM_BABY_LINDWORM];
	struct permonst* randombdragonf = &mons[PM_BABY_SARKANY];
	struct permonst* randombdragong = &mons[PM_BABY_SIRRUSH];
	struct permonst* randombdragonh = &mons[PM_BABY_LEVIATHAN];
	struct permonst* randombdragoni = &mons[PM_BABY_WYVERN];
	struct permonst* randombdragonj = &mons[PM_BABY_GLOWING_DRAGON];
	struct permonst* randombdragonk = &mons[PM_BABY_GUIVRE];

	struct permonst* pokshamblerl = &mons[PM_PUPURIN];
	struct permonst* pokshamblerla = &mons[PM_SAPUSAUR];
	struct permonst* pokshamblerm = &mons[PM_ODDOSHISHI];
	struct permonst* pokshamblerma = &mons[PM_TSUBOTSUBO];
	struct permonst* pokshamblerx = &mons[PM_OKUTAN];
	struct permonst* pokshamblerxa = &mons[PM_RATICLAW];
	struct permonst* pokshambler = &mons[PM_PSYBUR];
	struct permonst* pokshamblerp = &mons[PM_HARISEN];
	struct permonst* pokshamblert = &mons[PM_SUIKUN];
	struct permonst* pokshamblertp = &mons[PM_HOUOU];
	struct permonst* pokshamblers = &mons[PM_LOCUSTOD];
	struct permonst* pokshamblersp = &mons[PM_FORETOSU];	
	struct permonst* pokshamblerxts = &mons[PM_CHARCOLT];
	struct permonst* pokshamblerxtsp = &mons[PM_MILLENUM];

	struct attack* attkptr;
	int no_extra_food = FALSE;

	flags.female = flags.initgend;
	flags.beginner = 1;

	/* WAC -- Clear Tech List since adjabil will init the 1st level techs*/
	for (i = 0; i <= MAXTECH; i++) tech_list[i].t_id = NO_TECH;

	/* Initialize spells */
	for (i = 0; i <= MAXSPELL; i++) spl_book[i].sp_id = NO_SPELL;

	/* Initialize the "u" structure.
	 * Note that some values may have been incorrectly set by a failed restore.
	 */
	(void) memset((genericptr_t)&u, 0, sizeof(u));
	setustuck((struct monst *)0);
#if 0	/* documentation of more zero values as desirable */
	u.usick_cause[0] = 0;
	u.uluck  = u.moreluck = 0;
# ifdef TOURIST
	uarmu = 0;
# endif
	uarm = uarmc = uarmh = uarms = uarmg = uarmf = 0;
	uwep = uball = uchain = uleft = uright = 0;
	/* WAC Added uswapwep, uquiver*/
	uswapwep = uquiver = 0;
	/* KMH -- added two-weapon combat */
	u.twoweap = 0;
	u.ublessed = 0;				/* not worthy yet */
	u.ugangr   = 0;				/* gods not angry */
	u.ugifts   = 0;				/* no divine gifts bestowed */
	/* WAC not needed - use techs */
/*	u.unextuse = 0;
	u.ulastuse = 0;*/
# ifdef ELBERETH
	u.uevent.uhand_of_elbereth = 0;
# endif
	u.uevent.uheard_tune = 0;
	u.uevent.uopened_dbridge = 0;
	u.uevent.udemigod = 0;		/* not a demi-god yet... */
	u.udg_cnt = 0;
	u.mh = u.mhmax = Upolyd = 0;
	u.uz.dnum = u.uz0.dnum = 0;
	u.utotype = 0;
#endif	/* 0 */

	u.uz.dlevel = 1;
	u.uz0.dlevel = 0;
	u.utolev = u.uz;

	u.umoved = FALSE;
	u.umortality = 0;
	u.ugrave_arise = Role_if(PM_PIRATE) ? PM_SKELETAL_PIRATE : Role_if(PM_BINDER) ? PM_SHADE : NON_PM;
	
	u.ukinghill = 0;
	u.protean = 0;

	u.umonnum = u.umonster = (flags.female &&
			urole.femalenum != NON_PM) ? urole.femalenum :
			urole.malenum;

	init_uasmon();

	u.ulevel = 0;	/* set up some of the initial attributes */
	u.uhp = u.uhpmax = u.uhplast = newhp();
	u.uenmax = urole.enadv.infix + urace.enadv.infix;
	if (urole.enadv.inrnd > 0)
	    u.uenmax += rnd(urole.enadv.inrnd);
	if (urace.enadv.inrnd > 0)
	    u.uenmax += rnd(urace.enadv.inrnd);
	u.uen = u.uenmax;
	u.uspellprot = 0;
	adjabil(0,1);
	u.ulevel = u.ulevelmax = 1;

	init_uhunger();
	u.ublesscnt = rnz(Race_if(PM_HAXOR) ? 150 : 300);			/* no prayers just yet */
	u.weakcheat = 0;	/* increases if a character wearing a stat lock ring grows weak to prevent them from restoring strength points they didn't lose in the first place --Amy */
	u.stethocheat = 0;

      u.monstertimeout = rnz(10000)+rnz(15000); /*This NEEDS to be set here, because otherwise saving/restoring will set*/
      u.monstertimefinish = rnz(10000)+rnz(20000)+u.monstertimeout; /*a new value, allowing the player to cheat. --Amy*/
	u.legscratching = (Role_if(PM_BLEEDER) ? 3 : 1); /*must also be set here; this may increase over time*/
	u.next_check = rnz(600); /* it used to be exactly 600 at the start of every game */

	u.statuetrapname = rn2(NUMMONS);

	u.wormpolymorph = 0; /* if this is non-zero, the player's next polymorph will be into the specified monster */
	u.eeveelution = PM_VAPOREON; /* failsafe */
	if (u.monstertimefinish % 42 == 0) u.eeveelution = PM_VOLAREON;
	else if (u.monstertimefinish % 23 == 0) u.eeveelution = PM_INSECTEON;
	else if (u.monstertimefinish % 15 == 0) u.eeveelution = PM_SYLVEON;
	else if (u.monstertimefinish % 13 == 0) u.eeveelution = PM_GLACEON;
	else if (u.monstertimefinish % 11 == 0) u.eeveelution = PM_LEAFEON;
	else if (u.monstertimefinish % 10 == 0) u.eeveelution = PM_UMBREON;
	else if (u.monstertimefinish % 9 == 0) u.eeveelution = PM_ESPEON;
	else if (u.monstertimefinish % 7 == 0) u.eeveelution = PM_JOLTEON;
	else if (u.monstertimefinish % 3 == 0) u.eeveelution = PM_FLAREON;
	else u.eeveelution = PM_VAPOREON;

	u.urmaxlvl = 1; /* will go up if an asgardian race player levels up */
	u.urmaxlvlB = 1; /* will go up if a cyborg role player levels up */
	u.urmaxlvlC = 1; /* will go up if a binder role player levels up */
	u.urmaxlvlD = 1; /* will go up if a bard role player levels up */

	u.uhereticgodinit = 0; /* for heretic race */
	u.uhereticgodlawful = randrole(); /* for heretic race */
		    while (!roles[u.uhereticgodlawful].lgod)	/* unless they're missing */
			u.uhereticgodlawful = randrole(); /* for heretic race */
	u.uhereticgodneutral = randrole(); /* for heretic race */
		    while (!roles[u.uhereticgodneutral].lgod)	/* unless they're missing */
			u.uhereticgodneutral = randrole(); /* for heretic race */
	u.uhereticgodchaotic = randrole(); /* for heretic race */
		    while (!roles[u.uhereticgodchaotic].lgod)	/* unless they're missing */
			u.uhereticgodchaotic = randrole(); /* for heretic race */

/* In order to make the game even more interesting for lost souls, they cannot level teleport or branchport at all. */

	u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] = u.ualign.type =
			aligns[flags.initalign].value;
	u.ulycn = NON_PM;
#if defined(BSD) && !defined(POSIX_TYPES)
	(void) time((long *)&u.ubirthday);
#else
	(void) time(&u.ubirthday);
#endif
	/*
	 *  For now, everyone but elves, cavemen and lycanthropes starts
	 *  out with a night vision range of 1 and their xray range disabled.
	 */
	u.nv_range   =  1;
	u.xray_range = -1;

	/* Role-specific initializations */
	switch (Role_switch) {
	case PM_ARCHEOLOGIST:
		switch (rnd(5)) {   
		    case 1: Archeologist[A_BOOK].trotyp = SPE_DETECT_FOOD; break;
		    case 2: Archeologist[A_BOOK].trotyp = SPE_DETECT_MONSTERS; break;
		    case 3: Archeologist[A_BOOK].trotyp = SPE_LIGHT; break;
		    case 4: Archeologist[A_BOOK].trotyp = SPE_KNOCK; break;
		    case 5: Archeologist[A_BOOK].trotyp = SPE_WIZARD_LOCK; break;
		    default: break;
		}
		ini_inv(Archeologist);
		if(!rn2(4)) ini_inv(Blindfold);
#ifdef TOURIST
		else if(!rn2(4)) ini_inv(Towel);
		if(!rn2(4)) ini_inv(Leash);
#endif
		if(!rn2(4)) ini_inv(Tinopener);
		else if(!rn2(4))
		  (rn2(100) > 50 ? ini_inv(Lamp) : ini_inv(Torch));
		if(!rn2(8)) ini_inv(Magicmarker);
		knows_object(TOUCHSTONE);
		knows_object(SACK);
		skill_init(Skill_A);
		break;

	case PM_AUGURER:
		switch (rnd(5)) {   
		    case 1: Augurer[AUG_BOOK].trotyp = SPE_CAUSE_FEAR; break;
		    case 2: Augurer[AUG_BOOK].trotyp = SPE_CONFUSE_MONSTER; break;
		    case 3: Augurer[AUG_BOOK].trotyp = SPE_LIGHT; break;
		    case 4: Augurer[AUG_BOOK].trotyp = SPE_KNOCK; break;
		    case 5: Augurer[AUG_BOOK].trotyp = (!rn2(3) ? SPE_SLEEP : !rn2(2) ? SPE_IDENTIFY : SPE_MAGIC_MAPPING); break;
		    default: break;
		}
		ini_inv(Augurer);
		if(!rn2(4)) ini_inv(Candle);
		else if(!rn2(4)) ini_inv(Touchstone);
		else if(!rn2(4)) ini_inv(Magicmarker);

		skill_init(Skill_Aug);
		break;

	case PM_BARBARIAN:
		if (rn2(100) >= 50) {   /* see Elf comment */
		    Barbarian[B_MAJOR].trotyp = BATTLE_AXE;
		    Barbarian[B_MINOR].trotyp = SHORT_SWORD;
		}
		ini_inv(Barbarian);
		if(!rn2(6)) ini_inv(Torch);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_B);
		break;
	case PM_BINDER:
		ini_inv(Binder);
		skill_init(Skill_Bin);
	  u.ualign.sins += 16; /*One transgression is all it takes*/
	    change_luck(-1); /*One resurection or two rehumanizations is all it takes*/
		break;

	case PM_POLITICIAN:
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = (3000 + rnd(2000));
#else
		u.umoney0 = (3000 + rnd(2000));
#endif
		ini_inv(Politician);
		skill_init(Skill_Pol);


		break;

	case PM_BLEEDER:
		ini_inv(Bleeder);
		skill_init(Skill_Ble);
		break;
	case PM_LIBRARIAN:
		ini_inv(Librarian);
		skill_init(Skill_Lib);
		break;
	case PM_SUPERMARKET_CASHIER:

#ifndef GOLDOBJ
		u.ugold = u.ugold0 = 500;
#else
		u.umoney0 = 500;
#endif

		ini_inv(Supermarket);
		if ((rnd(5)) > 2) ini_inv(SupermarketB);
		if (!rn2(5)) ini_inv(SupermarketA);
		skill_init(Skill_Sup);
		break;
	case PM_INTEL_SCRIBE:
		ini_inv(Scribe);
		skill_init(Skill_Scr);
		break;
	case PM_GAMER:
		ini_inv(Gamer);
		skill_init(Skill_Gam);
		break;
	case PM_ARTIST:
		ini_inv(Artist);
		skill_init(Skill_Art);
		break;
	case PM_COOK:
		ini_inv(Cook);
		skill_init(Skill_Coo);
		break;
	case PM_BARD:
		if (rn2(100) >= 50) Bard[BARD_INSTR].trotyp = WOODEN_FLUTE;
		if (rn2(100) >= 85) Bard[BARD_WHISTLE].trotyp = BELL;
		Bard[BARD_BOOZE].trquan = rn1(2, 5);
		ini_inv(Bard);
		/* This depends on the order in objects.c */
		for (i = TIN_WHISTLE; i <= DRUM_OF_EARTHQUAKE; i++)
			knows_object(i);
		/* Bards know about the enchantment spellbooks, though they don't know
		   the spells */
		knows_object(SPE_SLEEP);
		knows_object(SPE_CONFUSE_MONSTER);
		knows_object(SPE_SLOW_MONSTER);
		knows_object(SPE_CAUSE_FEAR);
		knows_object(SPE_CHARM_MONSTER);
		/* Bards also know a lot about legendary & magical stuff. */
		know_random_obj();
		skill_init(Skill_Bard);
		break;
	case PM_CAVEMAN:
		u.nv_range = 2;
		Cave_man[C_AMMO].trquan = rn1(11, 10);	/* 10..20 */
		ini_inv(Cave_man);
		skill_init(Skill_C);
		break;
	case PM_GANGSTER:
		ini_inv(Gangster);
		skill_init(Skill_Gan);
		break;
	case PM_GUNNER:
		ini_inv(Gunner);
		skill_init(Skill_Gun);
		break;
	case PM_DEATH_EATER:
		ini_inv(Death_Eater);
		skill_init(Skill_Dea);
		break;
	case PM_POKEMON:
		ini_inv(Pokemon);
		skill_init(Skill_Pok);
		break;
	case PM_FLAME_MAGE:
		switch (rnd(2)) {                
			case 1: Flame_Mage[F_BOOK].trotyp = SPE_DETECT_MONSTERS; break;
			case 2: Flame_Mage[F_BOOK].trotyp = SPE_LIGHT; break;
			default: break;
		}
		ini_inv(Flame_Mage);
		if(!rn2(5)) ini_inv(Lamp);
		else if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(5)) ini_inv(Magicmarker);
		skill_init(Skill_F);
		break;
	case PM_COURIER:
        ini_inv(Courier);
        skill_init(Skill_Cou);
		break;

	case PM_SPACEWARS_FIGHTER:
        ini_inv(Spacewars_Fighter);
        skill_init(Skill_Spa);

#ifndef GOLDOBJ
		u.ugold = u.ugold0 = 1500;
#else
		u.umoney0 = 1500;
#endif
		break;

	case PM_ROCKER:
        ini_inv(Rocker);
        skill_init(Skill_Roc);
		break;

	case PM_ZYBORG:
        ini_inv(Zyborg);
        skill_init(Skill_Zyb);
		break;

	case PM_LUNATIC:
        ini_inv(Lunatic);
        skill_init(Skill_Lun);

		if (!Race_if(PM_AK_THIEF_IS_DEAD_)) {
		switch (rnd(7)) {
		case 1: 	    u.ulycn = PM_WEREWOLF; break;
		case 2: 	    u.ulycn = PM_WEREJACKAL; break;
		case 3: 	    u.ulycn = PM_WERERAT; break;
		case 4: 	    u.ulycn = PM_WEREPANTHER; break;
		case 5: 	    u.ulycn = PM_WERETIGER; break;
		case 6: 	    u.ulycn = PM_WERESNAKE; break;
		case 7: 	    u.ulycn = PM_WERESPIDER; break;
		default: 	    u.ulycn = PM_WEREWOLF; break;

		}
		}

		break;

#ifdef CONVICT
	case PM_CONVICT:
        ini_inv(Convict);
        knows_object(SKELETON_KEY);
        knows_object(GRAPPLING_HOOK);
        skill_init(Skill_Con);
	  u.ualign.sins += 16; /* You have sinned */
        u.uhunger = 200;  /* On the verge of hungry */
    	/* u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] =
        u.ualign.type = A_CHAOTIC; Override racial alignment */
        urace.hatemask |= urace.lovemask;   /* Hated by the race's allies */
        urace.lovemask = 0; /* Convicts are pariahs of their race */
	    change_luck(-1); /* both their alignment and luck start out negative */
        break;
#endif	/* CONVICT */
	case PM_HEALER:
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rn1(1000, 1001);
#else
		u.umoney0 = rn1(1000, 1001);
#endif
		ini_inv(Healer);
		knows_classX(POTION_CLASS); /* WAC - remove? */ /* edit by Amy - nope, but change to include magic ones */
		knows_object(POT_SICKNESS);
		knows_object(POT_BLINDNESS);
		knows_object(POT_HALLUCINATION);
		knows_object(POT_RESTORE_ABILITY);
		knows_object(POT_FULL_HEALING);
		knows_object(HEALTHSTONE);	/* KMH */
		if(!rn2(5)) ini_inv(Lamp);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
		skill_init(Skill_H);
		break;
	case PM_ICE_MAGE:
		switch (rnd(2)) {                
			case 1: Ice_Mage[I_BOOK].trotyp = SPE_CONFUSE_MONSTER; break;
			case 2: Ice_Mage[I_BOOK].trotyp = SPE_SLOW_MONSTER; break;
			default: break;
		}
		ini_inv(Ice_Mage);
		if(!rn2(5)) ini_inv(Lamp);
		else if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(5)) ini_inv(Magicmarker);
		skill_init(Skill_I);
		break;

	case PM_ELECTRIC_MAGE:
		ini_inv(Electric_Mage);
		if(!rn2(5)) ini_inv(Lamp);
		else if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(5)) ini_inv(Magicmarker);
		skill_init(Skill_Ele);
		break;

	case PM_CRUEL_ABUSER:
		ini_inv(Abuser);
		skill_init(Skill_Abu);
		break;

	case PM_MYSTIC:
		ini_inv(Mystic);
		skill_init(Skill_Mys);
		break;

	case PM_JESTER:
		ini_inv(Jester);
		skill_init(Skill_Jes);
		break;

	case PM_LADIESMAN:
		ini_inv(Ladiesman);
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = 300;
#else
		u.umoney0 = 300;
#endif
		skill_init(Skill_Lad);
		break;

	case PM_BLOODSEEKER:
		ini_inv(Bloodseeker);
		if ((rnd(10)) > 3) ini_inv(BloodseekerA);
		skill_init(Skill_Blo);
		break;

	case PM_SLAVE_MASTER:
		ini_inv(SlaveMaster);
		if ((rnd(10)) > 3) ini_inv(SlaveMasterA);
		if (!rn2(2)) ini_inv(SlaveMasterB);
		if ((rnd(100)) > 92) ini_inv(SlaveMasterC);
		skill_init(Skill_Sla);
		break;

	case PM_ACID_MAGE:
		ini_inv(Acid_Mage);
		if(!rn2(5)) ini_inv(Lamp);
		else if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(5)) ini_inv(Magicmarker);
		skill_init(Skill_Aci);
		break;

	case PM_GEEK:
		Geek[G_IC].trquan = rn2(7) + 1;
		ini_inv(Geek);
		skill_init(Skill_G);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		knows_object(PACK_OF_FLOPPIES);
		knows_object(POT_JOLT_COLA);
		knows_object(DIODE);
		knows_object(TRANSISTOR);
		knows_object(IC);
		break;

	case PM_GRADUATE:
		ini_inv(Graduate);
		skill_init(Skill_Gra);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		break;

	case PM_SCIENTIST:
		ini_inv(Scientist);
		skill_init(Skill_Sci);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		knows_object(CHEMISTRY_SET);
		knows_classX(POTION_CLASS);
		break;

#ifdef JEDI
	case PM_JEDI:
		ini_inv(Jedi);
#ifdef D_SABER
		switch(rnd(3)) {
			case 1: ini_inv(RedSaber); break;
			case 2: ini_inv(BlueSaber); break;
			case 3: ini_inv(GreenSaber); break;
			default: break;
		}
#else
		switch(rnd(2)) {
			case 1: ini_inv(RedSaber); break;
			case 2: ini_inv(GreenSaber); break;
			default: break;
		}
#endif
		if(!rn2(2)) ini_inv(Blindfold);
		skill_init(Skill_J);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		break;
#endif
	case PM_KNIGHT:
		ini_inv(Knight);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		/* give knights chess-like mobility
		 * -- idea from wooledge@skybridge.scl.cwru.edu */
		HJumping |= FROMOUTSIDE;
		skill_init(Skill_K);
		break;

	case PM_WANDKEEPER:
		ini_inv(Wandkeeper);
		knows_classX(WAND_CLASS);
		skill_init(Skill_Wan);
		break;

	case PM_FEAT_MASTER:
		ini_inv(StuntMaster);

		HJumping |= FROMOUTSIDE;
		skill_init(Skill_Stu);
		break;

	case PM_FOXHOUND_AGENT:
		ini_inv(FoxhoundAgent);

		skill_init(Skill_Fox);
		break;

	case PM_PALADIN:
		ini_inv(Paladin);

		skill_init(Skill_Pal);
		break;

	case PM_MONK:
		switch (rn2(90) / 30) {
		case 0: Monk[M_BOOK].trotyp = SPE_HEALING; break;
		case 1: Monk[M_BOOK].trotyp = SPE_PROTECTION; break;
		case 2: Monk[M_BOOK].trotyp = SPE_SLEEP; break;
		}
		ini_inv(Monk);
		if(!rn2(5)) ini_inv(Magicmarker);
		else if(!rn2(10)) ini_inv(Lamp);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_Mon);
		break;
	case PM_SAIYAN:
		ini_inv(Saiyan);
		skill_init(Skill_Sai);
		break;

	case PM_PSION:
		switch (rn2(90) / 30) {
		case 0: Psion[M_BOOK].trotyp = SPE_KNOCK; break;
		case 1: Psion[M_BOOK].trotyp = SPE_WIZARD_LOCK; break;
		case 2: Psion[M_BOOK].trotyp = SPE_CHARM_MONSTER; break;
		}
		ini_inv(Psion);
		if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(10)) ini_inv(Magicmarker);
		knows_classX(SPBOOK_CLASS);
		knows_classX(ARMOR_CLASS);
		skill_init(Skill_Psi);
		break;
	case PM_NOBLEMAN:
		if(flags.female){
			Noble[NOB_SHIRT].trotyp = VICTORIAN_UNDERWEAR;
		}
		ini_inv(Noble);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_Nob);
		break;
	case PM_ACTIVISTOR:
		if(flags.female){
			Activistor[ACT_SHIRT].trotyp = VICTORIAN_UNDERWEAR;
		}
		ini_inv(Activistor);
		skill_init(Skill_Act);
		break;
	case PM_PIRATE:
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rnd(300);
#else
		u.umoney0 = rnd(300);
#endif
		Pirate[PIR_KNIVES].trquan = rn1(2, 2);
		if(!rn2(4)) Pirate[PIR_SNACK].trotyp = KELP_FROND;
		Pirate[PIR_SNACK].trquan += rn2(4);
		if(rn2(100)<50)	Pirate[PIR_JEWELRY].trotyp = RIN_ADORNMENT;
		if(rn2(100)<50)	Pirate[PIR_TOOL].trotyp = GRAPPLING_HOOK;
		ini_inv(Pirate);
		knows_object(OILSKIN_SACK);
		knows_object(OILSKIN_CLOAK);
		knows_object(GRAPPLING_HOOK);
		skill_init(Skill_Pir);
		break;
	case PM_KORSAIR:
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rnd(300);
#else
		u.umoney0 = rnd(300);
#endif
		ini_inv(Korsair);
		knows_object(OILSKIN_SACK);
		knows_object(OILSKIN_CLOAK);
		knows_object(GRAPPLING_HOOK);
		skill_init(Skill_Kor);
		break;
	case PM_GLADIATOR:
		ini_inv(Gladiator);
		skill_init(Skill_Gla);
		knows_class(ARMOR_CLASS);
		knows_class(WEAPON_CLASS);
		break;
	case PM_GOFF:
		ini_inv(Goff);
		skill_init(Skill_Gof);
		break;
	case PM_AMAZON:
		ini_inv(Amazon);
		skill_init(Skill_Ama);
		knows_class(ARMOR_CLASS);
		knows_class(WEAPON_CLASS);
		break;
	case PM_ORDINATOR:
		ini_inv(Ordinator);
		skill_init(Skill_Ord);
		knows_class(ARMOR_CLASS);
		knows_class(WEAPON_CLASS);
		break;
	case PM_THALMOR:
		ini_inv(Thalmor);
		skill_init(Skill_Tha);
		knows_class(ARMOR_CLASS);
		knows_class(WEAPON_CLASS);
		break;
	case PM_ALTMER:
		ini_inv(Altmer);
		skill_init(Skill_Alt);
		knows_classX(SPBOOK_CLASS);
	    knows_object(ELVEN_SHORT_SWORD);
	    knows_object(ELVEN_ARROW);
	    knows_object(ELVEN_BOW);
	    knows_object(ELVEN_SPEAR);
	    knows_object(ELVEN_DAGGER);
	    knows_object(ELVEN_BROADSWORD);
	    knows_object(ELVEN_MITHRIL_COAT);
	    knows_object(ELVEN_LEATHER_HELM);
	    knows_object(ELVEN_SHIELD);
	    knows_object(ELVEN_BOOTS);
	    knows_object(ELVEN_CLOAK);
		break;
	case PM_BOSMER:
		ini_inv(Bosmer);
		skill_init(Skill_Bos);
	    knows_object(ELVEN_SHORT_SWORD);
	    knows_object(ELVEN_ARROW);
	    knows_object(ELVEN_BOW);
	    knows_object(ELVEN_SPEAR);
	    knows_object(ELVEN_DAGGER);
	    knows_object(ELVEN_BROADSWORD);
	    knows_object(ELVEN_MITHRIL_COAT);
	    knows_object(ELVEN_LEATHER_HELM);
	    knows_object(ELVEN_SHIELD);
	    knows_object(ELVEN_BOOTS);
	    knows_object(ELVEN_CLOAK);
		break;
	case PM_DUNMER:
		ini_inv(Dunmer);
		skill_init(Skill_Dun);
	    knows_object(ELVEN_SHORT_SWORD);
	    knows_object(ELVEN_ARROW);
	    knows_object(ELVEN_BOW);
	    knows_object(ELVEN_SPEAR);
	    knows_object(ELVEN_DAGGER);
	    knows_object(ELVEN_BROADSWORD);
	    knows_object(ELVEN_MITHRIL_COAT);
	    knows_object(ELVEN_LEATHER_HELM);
	    knows_object(ELVEN_SHIELD);
	    knows_object(ELVEN_BOOTS);
	    knows_object(ELVEN_CLOAK);
		break;
	case PM_DIVER:
		ini_inv(Diver);
		knows_object(OILSKIN_SACK);
		knows_object(OILSKIN_CLOAK);
		knows_object(GRAPPLING_HOOK);
		skill_init(Skill_Div);
		break;
	case PM_NECROMANCER:
		switch (rnd(5)) {   
                    case 1: Necromancer[N_BOOK].trotyp = SPE_FORCE_BOLT; break;
                    case 2: Necromancer[N_BOOK].trotyp = SPE_KNOCK; break;
                    case 3: Necromancer[N_BOOK].trotyp = SPE_MAGIC_MISSILE; break;
                    case 4: Necromancer[N_BOOK].trotyp = SPE_CREATE_MONSTER; break;
                    case 5: Necromancer[N_BOOK].trotyp = SPE_WIZARD_LOCK; break;
		    default: break;
		}
		ini_inv(Necromancer);
		knows_classX(SPBOOK_CLASS);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
		skill_init(Skill_N);
		break;
	case PM_WARRIOR:
		switch (rnd(20)) {   
                    case 1: ini_inv(WarXtrA); break;
                    case 2: ini_inv(WarXtrB); break;
                    case 3: ini_inv(WarXtrC); break;
                    case 4: ini_inv(WarXtrD); break;
                    case 5: ini_inv(WarXtrE); break;
                    case 6: ini_inv(WarXtrF); break;
                    case 7: ini_inv(WarXtrG); break;
                    case 8: ini_inv(WarXtrH); break;
                    case 9: ini_inv(WarXtrI); break;
                    case 10: ini_inv(WarXtrJ); break;
                    case 11: ini_inv(WarXtrK); break;
                    case 12: ini_inv(WarXtrL); break;
                    case 13: ini_inv(WarXtrM); break;
                    case 14: ini_inv(WarXtrN); break;
                    case 15: ini_inv(WarXtrO); break;
                    case 16: ini_inv(WarXtrP); break;
                    case 17: ini_inv(WarXtrQ); break;
                    case 18: ini_inv(WarXtrR); break;
                    case 19: ini_inv(WarXtrS); break;
                    case 20: ini_inv(WarXtrT); break;
		    default: break;
		}
		ini_inv(Warrior);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_War);
		break;

	case PM_DOLL_MISTRESS:
		ini_inv(DollMistress);

		skill_init(Skill_Dol);
		break;

	case PM_MAHOU_SHOUJO:
		ini_inv(MahouShoujo);

		skill_init(Skill_Mah);
		break;

	case PM_PRIEST:
		switch (rnd(9)) {   
		    case 1: Priest[P_BOOK].trotyp = SPE_FORCE_BOLT; break;
		    case 2: Priest[P_BOOK].trotyp = SPE_SLEEP; break;
		    case 3: Priest[P_BOOK].trotyp = SPE_RESIST_POISON; break;
		    case 4: Priest[P_BOOK].trotyp = SPE_RESIST_SLEEP; break;
		    case 5: Priest[P_BOOK].trotyp = SPE_DETECT_FOOD; break;
		    case 6: Priest[P_BOOK].trotyp = SPE_DETECT_MONSTERS; break;
		    case 7: Priest[P_BOOK].trotyp = SPE_LIGHT; break;
		    case 8: Priest[P_BOOK].trotyp = SPE_KNOCK; break;
		    case 9: Priest[P_BOOK].trotyp = SPE_WIZARD_LOCK; break;
		    default: break;
		}
		ini_inv(Priest);
		if(!rn2(10)) ini_inv(Magicmarker);
		else if(!rn2(10)) 
		  (rn2(100) > 50 ? ini_inv(Lamp) : ini_inv(Torch));
		knows_object(POT_WATER);
		spellbook_skill_raise(Skill_P, Priest[P_BOOK].trotyp);
		skill_init(Skill_P);
		/* KMH, conduct --
		 * Some may claim that this isn't agnostic, since they
		 * are literally "priests" and they have holy water.
		 * But we don't count it as such.  Purists can always
		 * avoid playing priests and/or confirm another player's
		 * role in their YAAP.
		 */
		break;
	case PM_CHEVALIER:
		ini_inv(Chevalier);
		if(!rn2(10)) ini_inv(Magicmarker);
		else if(!rn2(10)) 
		  (rn2(100) > 50 ? ini_inv(Lamp) : ini_inv(Torch));
		knows_object(POT_WATER);
		skill_init(Skill_Che);
		break;
	case PM_RANGER:
		Ranger[RAN_TWO_ARROWS].trquan = rn1(10, 50);
		Ranger[RAN_ZERO_ARROWS].trquan = rn1(10, 30);
		ini_inv(Ranger);
		skill_init(Skill_Ran);
		break;
	case PM_ELPH:
		Elph[ELP_TWO_ARROWS].trquan = rn1(10, 50);
		Elph[ELP_ZERO_ARROWS].trquan = rn1(10, 30);
		ini_inv(Elph);
		skill_init(Skill_Elp);

	    /* Elves can recognize all elvish objects */
	    knows_object(ELVEN_SHORT_SWORD);
	    knows_object(ELVEN_ARROW);
	    knows_object(ELVEN_BOW);
	    knows_object(ELVEN_SPEAR);
	    knows_object(ELVEN_DAGGER);
	    knows_object(ELVEN_BROADSWORD);
	    knows_object(ELVEN_MITHRIL_COAT);
	    knows_object(ELVEN_LEATHER_HELM);
	    knows_object(ELVEN_SHIELD);
	    knows_object(ELVEN_BOOTS);
	    knows_object(ELVEN_CLOAK);

		break;
	case PM_TRANSVESTITE:
		ini_inv(Transvestite);
		skill_init(Skill_Tra);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		break;
	case PM_TOPMODEL:
		ini_inv(Topmodel);
		skill_init(Skill_Top);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
	      u.uhunger = 200;  /* They don't eat much --Amy */
		break;

	case PM_BULLY:
		ini_inv(Bully);
		skill_init(Skill_Bul);

		if (rn2(5)) ini_inv(BullyA);
		if (rn2(2)) ini_inv(BullyB);
		if (!rn2(20)) ini_inv(Candle);
		if (!rn2(20)) ini_inv(BullyD);
		if (!rn2(20)) ini_inv(BullyE);

		break;

	case PM_PICKPOCKET:

		ini_inv(Pickpocket);
		skill_init(Skill_Pic);
		knows_classX(GEM_CLASS);
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = 200;
#else
		u.umoney0 = 200;
#endif

		break;

	case PM_ROGUE:
		Rogue[R_DAGGERS].trquan = rn1(10, 6);
		Rogue[R_DARTS].trquan = rn1(10, 25);
#ifdef FIREARMS
		if (rn2(100) < 30) {
			Rogue[R_DAGGERS].trotyp = PISTOL;
			Rogue[R_DAGGERS].trquan = 1;
			Rogue[R_DARTS].trotyp = BULLET;
		}
#endif
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rn1(500 ,1500);
#else
		u.umoney0 = rn1(500 ,1500);
#endif
		ini_inv(Rogue);
		if(!rn2(5)) ini_inv(Blindfold);
		knows_object(OILSKIN_SACK);
		skill_init(Skill_R);
		break;

	case PM_ASSASSIN:

		if (!rn2(20)) ini_inv(InvisibilityRing);
		ini_inv(Assassin);
		skill_init(Skill_Ass);
		break;

	case PM_NINJA:
		knows_class(WEAPON_CLASS);
		ini_inv(Ninja);
		skill_init(Skill_Nin);
		break;

	case PM_DRUNK:
		ini_inv(Drunk);
		skill_init(Skill_Dru);
		break;

	case PM_OFFICER:
		ini_inv(Officer);
		skill_init(Skill_Off);
		break;

	case PM_UNDERTAKER:
		ini_inv(Undertaker);
		skill_init(Skill_Unt);
		break;

	case PM_MUSICIAN:
		ini_inv(Musician);
		skill_init(Skill_Mus);
		break;

	case PM_ZOOKEEPER:
		ini_inv(Zookeeper);
		skill_init(Skill_Zoo);
		break;

	case PM_FIREFIGHTER:
		ini_inv(Firefighter);
		skill_init(Skill_Fir);
		break;

	case PM_LOCKSMITH:
		ini_inv(Locksmith);
		skill_init(Skill_Loc);
		break;

	case PM_SAMURAI:
		Samurai[S_ARROWS].trquan = rn1(20, 26);
		ini_inv(Samurai);
		if(!rn2(5)) ini_inv(Blindfold);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_S);
		break;

	case PM_OTAKU:
		ini_inv(Otaku);
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rnd(500);
#else
		u.umoney0 = rnd(500);
#endif

		skill_init(Skill_Ota);
		break;

#ifdef TOURIST
	case PM_TOURIST:
		Tourist[T_DARTS].trquan = rn1(20, 21);
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rn1(500,1000);
#else
		u.umoney0 = rn1(500,1000);
#endif
		ini_inv(Tourist);
		if(!rn2(25)) ini_inv(Tinopener);
		else if(!rn2(25)) ini_inv(Leash);
		else if(!rn2(25)) ini_inv(Towel);
		else if(!rn2(25)) ini_inv(Magicmarker);
		skill_init(Skill_T);
		break;
#endif /* TOURIST */
	case PM_UNDEAD_SLAYER:
		switch (rn2(100) / 25) {
		    case 0:	/* Pistol and silver bullets */
#ifdef FIREARMS
			UndeadSlayer[U_MINOR].trotyp = PISTOL;
			UndeadSlayer[U_RANGE].trotyp = SILVER_BULLET;
			UndeadSlayer[U_RANGE].trquan = rn1(10, 30);
			break;
#endif
		    case 1:	/* Crossbow and bolts */
			UndeadSlayer[U_MINOR].trotyp = CROSSBOW;
			UndeadSlayer[U_RANGE].trotyp = CROSSBOW_BOLT;
			UndeadSlayer[U_RANGE].trquan = rn1(10, 30);
			UndeadSlayer[U_MISC].trotyp = LOW_BOOTS;
			UndeadSlayer[U_MISC].trspe = 1;
			UndeadSlayer[U_ARMOR].trotyp = LEATHER_JACKET;
			UndeadSlayer[U_ARMOR].trspe = 1;
			/* helmet & armour are no longer candidates for
			 * substitution for orcish versions so no extra
			 * food should be given in compensation.
			 */
			if (Race_if(PM_ORC))
			    no_extra_food = TRUE;
		        break;
		    case 2:	/* Whip and daggers */
		        UndeadSlayer[U_MINOR].trotyp = BULLWHIP;
		        UndeadSlayer[U_MINOR].trspe = 2;
		        break;
		    case 3:	/* Silver spear and daggers */
			break;
		}
		ini_inv(UndeadSlayer);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		if(!rn2(6)) ini_inv(Lamp);
		skill_init(Skill_U);
		break;
	case PM_VALKYRIE:
		ini_inv(Valkyrie);
		if(!rn2(6)) 
		  (rn2(100) > 50 ? ini_inv(Lamp) : ini_inv(Torch));
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_V);
		break;
	case PM_WIZARD:
		switch (rnd(2)) {                
		    case 1: Wizard[W_BOOK1].trotyp = SPE_FORCE_BOLT; break;
		    case 2: Wizard[W_BOOK1].trotyp = SPE_SLEEP; break;
		    default: break;
		}
		switch (rnd(2)) {
		    case 1: Wizard[W_BOOK2].trotyp = SPE_RESIST_POISON; break;
		    case 2: Wizard[W_BOOK2].trotyp = SPE_RESIST_SLEEP; break;
		    default: break;
		}
		switch (rnd(5)) {   
		    case 1: Wizard[W_BOOK3].trotyp = SPE_DETECT_FOOD; break;
		    case 2: Wizard[W_BOOK3].trotyp = SPE_DETECT_MONSTERS; break;
		    case 3: Wizard[W_BOOK3].trotyp = SPE_LIGHT; break;
		    case 4: Wizard[W_BOOK3].trotyp = SPE_KNOCK; break;
		    case 5: Wizard[W_BOOK3].trotyp = SPE_WIZARD_LOCK; break;
		    default: break;
		}
		switch (rnd(9)) {
		    case 1: Wizard[W_BOOK4].trotyp = SPE_MAGIC_MISSILE; break;
		    case 2: Wizard[W_BOOK4].trotyp = SPE_CONFUSE_MONSTER; break;
		    case 3: Wizard[W_BOOK4].trotyp = SPE_SLOW_MONSTER; break;
		    case 4: Wizard[W_BOOK4].trotyp = SPE_CURE_BLINDNESS; break;
		    case 5: Wizard[W_BOOK4].trotyp = SPE_ENDURE_HEAT; break;
		    case 6: Wizard[W_BOOK4].trotyp = SPE_ENDURE_COLD; break;
		    case 7: Wizard[W_BOOK4].trotyp = SPE_INSULATE; break;
		    case 8: Wizard[W_BOOK4].trotyp = SPE_CREATE_MONSTER; break;
		    case 9: Wizard[W_BOOK4].trotyp = SPE_HEALING; break;
		    default: break;
		}
		ini_inv(Wizard);
		knows_classX(SPBOOK_CLASS);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
		skill_init(Skill_W);
		break;

	case PM_SAGE:

		ini_inv(Sage);
		if(!rn2(3)) ini_inv(Magicmarker);

		skill_init(Skill_Sag);
		break;

#ifdef YEOMAN
	case PM_YEOMAN:
		ini_inv(Yeoman);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_Y);
		break;
#endif

	default:	/* impossible */
		break;
	}

	/*** Race-specific initializations ***/
	switch (Race_switch) {
	case PM_HUMAN:
	    /* Nothing special */
	    break;

	case PM_ELF:
	    /*
	     * Elves are people of music and song, or they are warriors.
	     * Non-warriors get an instrument.  We use a kludge to
	     * get only non-magic instruments.
	     */
	    if (Role_if(PM_PRIEST) || Role_if(PM_WIZARD)) {
		static int trotyp[] = {
		    WOODEN_FLUTE, TOOLED_HORN, WOODEN_HARP,
		    BELL, BUGLE, LEATHER_DRUM
		};
		Instrument[0].trotyp = trotyp[rn2(SIZE(trotyp))];
		ini_inv(Instrument);
	    }

	    /* Elves can recognize all elvish objects */
	    knows_object(ELVEN_SHORT_SWORD);
	    knows_object(ELVEN_ARROW);
	    knows_object(ELVEN_BOW);
	    knows_object(ELVEN_SPEAR);
	    knows_object(ELVEN_DAGGER);
	    knows_object(ELVEN_BROADSWORD);
	    knows_object(ELVEN_MITHRIL_COAT);
	    knows_object(ELVEN_LEATHER_HELM);
	    knows_object(ELVEN_SHIELD);
	    knows_object(ELVEN_BOOTS);
	    knows_object(ELVEN_CLOAK);
	    break;
	case PM_DROW:
	    /* Drows can recognize all droven objects */
	    knows_object(DARK_ELVEN_SHORT_SWORD);
	    knows_object(DARK_ELVEN_ARROW);
	    knows_object(DARK_ELVEN_BOW);
	    knows_object(DARK_ELVEN_DAGGER);
	    knows_object(DARK_ELVEN_MITHRIL_COAT);
	    break;

	case PM_DWARF:
	    /* Dwarves can recognize all dwarvish objects */
	    knows_object(DWARVISH_SPEAR);
	    knows_object(DWARVISH_SHORT_SWORD);
	    knows_object(DWARVISH_MATTOCK);
	    knows_object(DWARVISH_IRON_HELM);
	    knows_object(DWARVISH_MITHRIL_COAT);
	    knows_object(DWARVISH_CLOAK);
	    knows_object(DWARVISH_ROUNDSHIELD);
	    break;

	case PM_GNOME:
		 /* Gnomes don't see how silly they really look */
		 knows_object(GNOMISH_HELM);
		 knows_object(GNOMISH_BOOTS);
		 knows_object(GNOMISH_SUIT);
		 knows_object(AKLYS);			  /* they're all carrying one! */
	    break;
	case PM_HUMAN_WEREWOLF:
	    if (!Role_if(PM_LUNATIC)) u.ulycn = PM_WEREWOLF;
/*	    u.nv_range = 2;
	    u.uen = u.uenmax += 6;
	    ini_inv(Lycanthrope);*/
	    break;

	case PM_HUMANOID_ANGEL:

	    u.nv_range = 2;
	    break;

	case PM_AK_THIEF_IS_DEAD_:

	    u.ulycn = PM_UN_IN_PROTECT_MODE;
	    break;

	case PM_LICH_WARRIOR:

		ini_inv(LichOne);
		ini_inv(LichTwo);

	   break;
	case PM_HUMAN_WRAITH:

		ini_inv(WraithItem);

	   break;

	case PM_ANGBANDER:

		ini_inv(AngbandItem);
		ini_inv(AngbandItemB);

	   break;

	case PM_ORC:
	    /* compensate for generally inferior equipment */
	    if (!no_extra_food && !Role_if(PM_WIZARD) &&
		    !Role_if(PM_FLAME_MAGE) && !Role_if(PM_ICE_MAGE) &&
		    !Role_if(PM_NECROMANCER))
#ifdef CONVICT
        if (!Role_if(PM_CONVICT))
#endif /* CONVICT */
		ini_inv(Xtra_food);
	    /* Orcs can recognize all orcish objects */
	    knows_object(ORCISH_SHORT_SWORD);
	    knows_object(ORCISH_ARROW);
	    knows_object(ORCISH_BOW);
	    knows_object(ORCISH_SPEAR);
	    knows_object(ORCISH_DAGGER);
	    knows_object(ORCISH_CHAIN_MAIL);
	    knows_object(ORCISH_RING_MAIL);
	    knows_object(ORCISH_HELM);
	    knows_object(ORCISH_SHIELD);
	    knows_object(URUK_HAI_SHIELD);
	    knows_object(ORCISH_CLOAK);
	    break;
	case PM_VAMPIRE:
	    /* Vampires start off with gods not as pleased, luck penalty */
	    adjalign(-5); 
	    u.ualign.sins += 5;
	    change_luck(-1);
		{
		static int trotyp[] = {POT_BLOOD, POT_VAMPIRE_BLOOD};
		Xtra_food[0].trotyp = trotyp[rn2(SIZE(trotyp))];
		ini_inv(Xtra_food);
	    }
		ini_inv(XtraRing);

		if(!rn2(5))
		{
		static int trotyp[] = {POT_BLOOD, POT_VAMPIRE_BLOOD};
		Xtra_fopod[0].trotyp = trotyp[rn2(SIZE(trotyp))];
		ini_inv(Xtra_fopod);
	    }
	    break;
	case PM_ALIEN: /* this is the harder than hard race, combine it with convict role for maximum torture */
	    adjalign(-20); 
	    change_luck(-2);
          ini_inv(AlienItem); break; /* yeah its a lodestone, good luck getting rid of it! */

	case PM_KOP:
          ini_inv(KopItemA); ini_inv(KopItemB); break;

	case PM_CURSER: /* starts out with cursed stuff --Amy */
		{register struct obj *obj;

		for(obj = invent; obj ; obj = obj->nobj)
			if (rn2(5))	curse(obj);
		break;
		}
	case PM_KOBOLT:
          ini_inv(KoboltItem);		
          ini_inv(KoboltItemB);		
          ini_inv(KoboltItemC);		
		HSleeping = 5;
		break;
	case PM_GASTLY:
          ini_inv(GhastFood);		
		break;
	case PM_INSECTOID:
          ini_inv(InsectoidItem);		
		break;
	case PM_NAVI:
          ini_inv(NaviItem);		
		break;
	case PM_UNGENOMOLD:
          ini_inv(UngMoldWand);		
		break;
	case PM_MAIA:
          ini_inv(MaiaWand);		
		break;
	case PM_CLOCKWORK_AUTOMATON:
          ini_inv(AutomatonItem);		
		break;
	case PM_TROLLOR:
		switch (rnd(4)) {   
                case 1: ini_inv(TrollItemA); break;
                case 2: ini_inv(TrollItemB); break;
                case 3: ini_inv(TrollItemC); break;
                case 4: ini_inv(TrollItemD); break;
		    default: break;
		}
	case PM_OGRO:
          ini_inv(OgroItem);		
		break;
	case PM_BATMAN:
          ini_inv(BatmanItem);		
		break;

	case PM_HUMANLIKE_DRAGON:

		switch (rnd(11)) {
                    case 1: ini_inv(WarXtrJ); break;
                    case 2: ini_inv(WarXtrK); break;
                    case 3: ini_inv(WarXtrL); break;
                    case 4: ini_inv(WarXtrM); break;
                    case 5: ini_inv(WarXtrN); break;
                    case 6: ini_inv(WarXtrO); break;
                    case 7: ini_inv(WarXtrP); break;
                    case 8: ini_inv(WarXtrQ); break;
                    case 9: ini_inv(WarXtrR); break;
                    case 10: ini_inv(WarXtrS); break;
                    case 11: ini_inv(WarXtrT); break;
		}
		break;
	case PM_GIGANT:
          ini_inv(GigantItem);		
		break;

	default:	/* impossible */
		break;
	}

		/* Players will start with a few healing and identify items now. --Amy */
          ini_inv(AlwaysStartItem);
          ini_inv(AlwaysStartItemB);
          ini_inv(AlwaysStartItemC); /* and one that restores mana */
          ini_inv(AlwaysStartItemD); /* and a curing scroll for those pesky delayed instadeaths */

	if (!strncmpi(plname, "lostsoul", 8)) { 
          ini_inv(LostSoulItem);  /* In Angband or TOME these would be scrolls of cure hunger instead of food rations. */
          ini_inv(LostSoulItemX);
          ini_inv(LostSoulItemY);
	} 

	if (!strncmpi(plname, "uberlostsoul", 12)) { /* lots of items, but you're gonna need them! --Amy */
          ini_inv(LostSoulItem);
          ini_inv(LostSoulItemX);
          ini_inv(LostSoulItemY);
          ini_inv(UberLostSoulItemA);
          ini_inv(UberLostSoulItemB);
          if (!rn2(20)) ini_inv(UberLostSoulItemC);
          if (!rn2(20)) ini_inv(UberLostSoulItemD);
          if (!rn2(25)) ini_inv(UberLostSoulItemE);
          if (!rn2(50)) ini_inv(UberLostSoulItemF);
          if (!rn2(33)) ini_inv(UberLostSoulItemG);
          if (!rn2(75)) ini_inv(UberLostSoulItemH);
          if (!rn2(100)) ini_inv(UberLostSoulItemI);
          if (!rn2(40)) ini_inv(UberLostSoulItemJ);
          if (!rn2(5)) ini_inv(UberLostSoulItemK);
          if (!rn2(25)) ini_inv(UberLostSoulItemL);
          if (!rn2(10)) ini_inv(UberLostSoulItemM);
          if (!rn2(5)) ini_inv(UberLostSoulItemN);
          if (!rn2(100)) ini_inv(UberLostSoulItemO);
          if (!rn2(100)) ini_inv(UberLostSoulItemP);
          if (!rn2(500)) ini_inv(UberLostSoulItemQ);
          if (!rn2(20)) ini_inv(UberLostSoulItemR);
          if (!rn2(40)) ini_inv(UberLostSoulItemR);
          if (!rn2(80)) ini_inv(UberLostSoulItemR);
          if (!rn2(160)) ini_inv(UberLostSoulItemR);
          if (!rn2(320)) ini_inv(UberLostSoulItemR);
          if (!rn2(640)) ini_inv(UberLostSoulItemR);
          if (!rn2(1280)) ini_inv(UberLostSoulItemR);
	} 
	/* Even if you actually get most of these items (by being lucky or savescumming), your level 1 character probably
	 * won't stand a chance on dlvl 64 with no means of levelporting or branchporting, so this is still fair. --Amy */

	knows_class(VENOM_CLASS);

	if (discover)
		ini_inv(Wishing);

#ifdef WIZARD
	if (wizard)
		read_wizkit();
#endif

#ifndef GOLDOBJ
	u.ugold0 += hidden_gold();	/* in case sack has gold in it */
#else
	if (u.umoney0) ini_inv(Money);
	u.umoney0 += hidden_gold();	/* in case sack has gold in it */
#endif

	/* Starting stat point totals will be higher or lower depending on the player character. --Amy */

	rolebounus = racebounus = genderbounus = alignbounus = 0;
	maxbounus = 90;
	if (Role_if(PM_BINDER)) maxbounus += 5;
	if (Role_if(PM_LUNATIC)) maxbounus += 5;
	if (Race_if(PM_HUMAN_WEREWOLF)) maxbounus += 5;
	if (Race_if(PM_SUXXOR)) maxbounus += 10;
	if (Race_if(PM_AK_THIEF_IS_DEAD_)) maxbounus += 5;

	switch (Role_switch) {

	case PM_ARCHEOLOGIST: rolebounus = rnd(15); break;
	case PM_BINDER: rolebounus = rnd(10); break;
	case PM_CAVEMAN: rolebounus = rnd(15); break;
	case PM_GEEK: rolebounus = rnd(18); break;
	case PM_TOURIST: rolebounus = rnd(15); break;
	case PM_SCIENTIST: rolebounus = rnd(15); break;
	case PM_PIRATE: rolebounus = rnd(22); break;
	case PM_JEDI: rolebounus = rnd(22); break;
	case PM_TOPMODEL: rolebounus = rnd(15); break;
	case PM_UNDEAD_SLAYER: rolebounus = rnd(25); break;
	case PM_COURIER: rolebounus = rnd(5); break;
	case PM_ZYBORG: rolebounus = rnd(30); break;
	case PM_LUNATIC: rolebounus = rnd(30); break;
	case PM_BLEEDER: rolebounus = rnd(30); break;
	default:	rolebounus = rnd(20); break;

	}

	switch (Race_switch) {

	case PM_ALIEN: racebounus = rnz(2); break;
	case PM_CURSER: racebounus = rnz(12); break;
	case PM_DOPPELGANGER: racebounus = rnz(8); break;
	case PM_DROW: racebounus = rnz(7); break;
	case PM_BRETON: racebounus = rnz(5); break;
	case PM_REDGUARD: racebounus = rnz(5); break;
	case PM_IMPERIAL: racebounus = rnz(13); break;
	case PM_ELF: racebounus = rnz(11); break;
	case PM_GIGANT: racebounus = rnz(12); break;
	case PM_HOBBIT: racebounus = rnz(8); break;
	case PM_KOBOLT: racebounus = rnz(8); break;
	case PM_HUMAN_WEREWOLF: racebounus = rnz(15); break;
	case PM_AK_THIEF_IS_DEAD_: racebounus = rnz(15); break;
	case PM_MOULD: racebounus = rnz(12); break;
	case PM_DEATHMOLD: racebounus = rnz(12); break;
	case PM_MISSINGNO: racebounus = rnz(20); break;
	case PM_WORM_THAT_WALKS: racebounus = rnz(13); break;
	case PM_OGRO: racebounus = rnz(11); break;
	case PM_TROLLOR: racebounus = rnz(12); break;
	case PM_UNGENOMOLD: racebounus = rnz(15); break;
	case PM_VAMPIRE: racebounus = rnz(12); break;
	case PM_DWARF: racebounus = rnz(11); break;
	case PM_ASGARDIAN: racebounus = rnz(15); break;
	case PM_HAXOR: racebounus = rnz(15); break;
	case PM_SUXXOR: racebounus = rnz(16); break;
	default:	racebounus = rnz(10); break;

	}

    switch (u.ualign.type) {
    case A_LAWFUL: alignbounus = rnz(5); break;
    case A_NEUTRAL: alignbounus = rnd(12); break;
    case A_CHAOTIC: alignbounus = rnz(3); break;
	default: /*impossible*/ alignbounus = rnz(3); break;

	}

	if (flags.female) genderbounus = rnz(3);
	else genderbounus = rnd(5);

	temp = 30 + rolebounus + racebounus + alignbounus + genderbounus; /* new algorithm --Amy */
	if (temp > maxbounus) temp = maxbounus; /* prevent value from randomly becoming too high */
	/*temp = rn1(10,70);*/
	init_attr(temp);		/* init attribute values */
	find_ac();			/* get initial ac value */
	max_rank_sz();			/* set max str size for class ranks */
/*
 *	Do we really need this?
 */
	for(i = 0; i < A_MAX; i++)
	    if(!rn2(20)) {
		register int xd = rn2(7) - 2;	/* biased variation */
		(void) adjattrib(i, xd, TRUE);
		if (ABASE(i) < AMAX(i)) AMAX(i) = ABASE(i);
	    }
	/* make sure you can carry all you have - especially for Tourists */
	/* but also prevent players from having ridiculous starting strength or constitution, especially for ghasts --Amy */
	while (inv_weight() > 0 && ( (weight_cap() < 1000 && rn2(10)) || (weight_cap() > 999 && rn2(2)) ) ) {

		if (ABASE(A_STR) < 18) {
			if (adjattrib(A_STR, 1, TRUE)) continue;
		}
		if (ABASE(A_CON) < 18) {
			if (adjattrib(A_CON, 1, TRUE)) continue;
		}
		/* only get here when didn't boost strength or constitution */
		break;
	}

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(12)-3;*/				/* shuffle level */
	shambler->mmove = rn2(10)+9;				/* slow to very fast */
	shambler->ac = rn2(21)-10;				/* any AC */
	shambler->mr = rn2(5)*25;				/* varying amounts of MR */
	shambler->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shambler->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shambler->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shambler->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shambler->cnutrit = 20;					/* see above */
	shambler->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shambler->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shambler->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shambler->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shambler->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shambler->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shambler->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shambler->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shambler->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shambler->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shambler->mflags2 |= (1 << rn2(31));
	}
	shambler->mflags2 &= ~M2_MERC;				/* no guards */
	shambler->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shambler->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shambler->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*shambler->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerp->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerp->ac = rn2(21)-10;				/* any AC */
	shamblerp->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblerp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerp->cnutrit = 20;					/* see above */
	shamblerp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerp->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerp->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerp->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerp->mflags2 |= (1 << rn2(31));
	}
	shamblerp->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerp->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(12)-3;*/				/* shuffle level */
	pokshambler->mmove = rn2(10)+9;				/* slow to very fast */
	pokshambler->ac = rn2(21)-10;				/* any AC */
	pokshambler->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshambler->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &pokshambler->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	pokshambler->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshambler->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshambler->cnutrit = 20;					/* see above */
	pokshambler->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshambler->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshambler->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshambler->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshambler->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshambler->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshambler->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshambler->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshambler->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshambler->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshambler->mflags2 |= (1 << rn2(31));
	}
	pokshambler->mflags2 &= ~M2_MERC;				/* no guards */
	pokshambler->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshambler->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshambler->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*pokshambler->mlevel += rnd(12)-3;*/				/* shuffle level */
	pokshamblerp->mmove = rn2(10)+9;				/* slow to very fast */
	pokshamblerp->ac = rn2(21)-10;				/* any AC */
	pokshamblerp->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &pokshamblerp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	pokshamblerp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerp->cnutrit = 20;					/* see above */
	pokshamblerp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerp->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerp->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerp->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerp->mflags2 |= (1 << rn2(31));
	}
	pokshamblerp->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerp->mflags2 &= ~M2_PNAME;				/* not a proper name */



	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblera->mmove = rn2(10)+9;				/* slow to very fast */
	shamblera->ac = rn2(21)-10;				/* any AC */
	shamblera->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblera->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblera->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblera->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblera->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblera->cnutrit = 20;					/* see above */
	shamblera->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblera->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblera->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblera->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblera->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblera->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblera->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblera->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblera->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblera->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblera->mflags2 |= (1 << rn2(31));
	}
	shamblera->mflags2 &= ~M2_MERC;				/* no guards */
	shamblera->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblera->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblera->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*shambler->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerap->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerap->ac = rn2(21)-10;				/* any AC */
	shamblerap->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerap->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerap->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblerap->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerap->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerap->cnutrit = 20;					/* see above */
	shamblerap->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerap->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerap->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerap->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerap->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerap->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerap->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerap->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerap->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerap->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerap->mflags2 |= (1 << rn2(31));
	}
	shamblerap->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerap->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerap->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerap->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerb->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerb->ac = rn2(21)-10;				/* any AC */
	shamblerb->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerb->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerb->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblerb->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerb->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerb->cnutrit = 20;					/* see above */
	shamblerb->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerb->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerb->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerb->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerb->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerb->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblerb->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	shamblerb->mflags1 &= ~M1_CARNIVORE;				/* must be herbivore */

	shamblerb->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerb->mflags2 |= (1 << rn2(31));
	}
	shamblerb->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerb->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerb->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerb->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*shambler->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerbp->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerbp->ac = rn2(21)-10;				/* any AC */
	shamblerbp->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerbp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerbp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblerbp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerbp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerbp->cnutrit = 20;					/* see above */
	shamblerbp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerbp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerbp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerbp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerbp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerb->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblerbp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	shamblerb->mflags1 &= ~M1_CARNIVORE;				/* must be herbivore */

	shamblerbp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerbp->mflags2 |= (1 << rn2(31));
	}
	shamblerbp->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerbp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerbp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerbp->mflags2 &= ~M2_PNAME;				/* not a proper name */


	/*extra hard versions*/

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(24)-6;*/				/* shuffle level */
	shamblers->mmove = rn2(18)+9;				/* slow to very fast */
	shamblers->ac = rn2(35)-24;				/* any AC */
	shamblers->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblers->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(6); i++) {
		attkptr = &shamblers->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(20)+2;				/* either too high or too low */
	}
	shamblers->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblers->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblers->cnutrit = 20;					/* see above */
	shamblers->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblers->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblers->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblers->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblers->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblers->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblers->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblers->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblers->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblers->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblers->mflags2 |= (1 << rn2(31));
	}
	shamblers->mflags2 &= ~M2_MERC;				/* no guards */
	shamblers->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblers->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblers->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*shambler->mlevel += rnd(24)-6;*/				/* shuffle level */
	shamblersp->mmove = rn2(18)+9;				/* slow to very fast */
	shamblersp->ac = rn2(35)-24;				/* any AC */
	shamblersp->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblersp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(6); i++) {
		attkptr = &shamblersp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(20)+2;				/* either too high or too low */
	}
	shamblersp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblersp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblersp->cnutrit = 20;					/* see above */
	shamblersp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblersp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblersp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblersp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblersp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblersp->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblersp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblersp->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblersp->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblersp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblersp->mflags2 |= (1 << rn2(31));
	}
	shamblersp->mflags2 &= ~M2_MERC;				/* no guards */
	shamblersp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblersp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblersp->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(24)-6;*/				/* shuffle level */
	pokshamblers->mmove = rn2(18)+9;				/* slow to very fast */
	pokshamblers->ac = rn2(35)-24;				/* any AC */
	pokshamblers->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblers->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(6); i++) {
		attkptr = &pokshamblers->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(20)+2;				/* either too high or too low */
	}
	pokshamblers->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblers->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblers->cnutrit = 20;					/* see above */
	pokshamblers->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblers->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblers->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblers->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblers->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblers->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblers->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblers->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblers->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblers->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblers->mflags2 |= (1 << rn2(31));
	}
	pokshamblers->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblers->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblers->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblers->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*pokshambler->mlevel += rnd(24)-6;*/				/* shuffle level */
	pokshamblersp->mmove = rn2(18)+9;				/* slow to very fast */
	pokshamblersp->ac = rn2(35)-24;				/* any AC */
	pokshamblersp->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblersp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(6); i++) {
		attkptr = &pokshamblersp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(20)+2;				/* either too high or too low */
	}
	pokshamblersp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblersp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblersp->cnutrit = 20;					/* see above */
	pokshamblersp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblersp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblersp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblersp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblersp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblersp->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblersp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblersp->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblersp->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblersp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblersp->mflags2 |= (1 << rn2(31));
	}
	pokshamblersp->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblersp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblersp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblersp->mflags2 &= ~M2_PNAME;				/* not a proper name */


	/* what a horrible night to have a curse */
	/*pokshamblerxt->mlevel += rnd(24)-6;*/				/* shuffle level */
	pokshamblerxts->mmove = rn2(20)+9;				/* slow to very fast */
	pokshamblerxts->ac = rn2(40)-29;				/* any AC */
	pokshamblerxts->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerxts->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < (rnd(5) + 1); i++) {
		attkptr = &pokshamblerxts->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(24)+2;				/* either too high or too low */
	}
	pokshamblerxts->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerxts->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerxts->cnutrit = 20;					/* see above */
	pokshamblerxts->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerxts->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerxts->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerxts->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerxts->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerxts->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerxts->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerxts->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerxts->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerxts->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerxts->mflags2 |= (1 << rn2(31));
	}
	pokshamblerxts->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerxts->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerxts->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerxts->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*pokshamblerxt->mlevel += rnd(24)-6;*/				/* shuffle level */
	pokshamblerxtsp->mmove = rn2(20)+9;				/* slow to very fast */
	pokshamblerxtsp->ac = rn2(40)-29;				/* any AC */
	pokshamblerxtsp->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerxtsp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < (rnd(5) + 1); i++) {
		attkptr = &pokshamblerxtsp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(24)+2;				/* either too high or too low */
	}
	pokshamblerxtsp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerxtsp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerxtsp->cnutrit = 20;					/* see above */
	pokshamblerxtsp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerxtsp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerxtsp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerxtsp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerxtsp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerxtsp->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerxtsp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerxtsp->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerxtsp->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerxtsp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerxtsp->mflags2 |= (1 << rn2(31));
	}
	pokshamblerxtsp->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerxtsp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerxtsp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerxtsp->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshamblerxt->mlevel += rnd(24)-6;*/				/* shuffle level */
	shamblerz->mmove = rn2(20)+9;				/* slow to very fast */
	shamblerz->ac = rn2(40)-29;				/* any AC */
	shamblerz->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerz->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < (rnd(5) + 1); i++) {
		attkptr = &shamblerz->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(24)+2;				/* either too high or too low */
	}
	shamblerz->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerz->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerz->cnutrit = 20;					/* see above */
	shamblerz->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerz->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerz->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerz->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerz->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerz->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerz->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerz->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerz->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerz->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerz->mflags2 |= (1 << rn2(31));
	}
	shamblerz->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerz->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerz->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerz->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*pokshamblerxt->mlevel += rnd(24)-6;*/				/* shuffle level */
	shamblerza->mmove = rn2(20)+9;				/* slow to very fast */
	shamblerza->ac = rn2(40)-29;				/* any AC */
	shamblerza->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerza->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < (rnd(5) + 1); i++) {
		attkptr = &shamblerza->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(24)+2;				/* either too high or too low */
	}
	shamblerza->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerza->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerza->cnutrit = 20;					/* see above */
	shamblerza->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerza->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerza->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerza->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerza->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerza->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerza->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerza->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerza->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerza->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerza->mflags2 |= (1 << rn2(31));
	}
	shamblerza->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerza->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerza->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerza->mflags2 &= ~M2_PNAME;				/* not a proper name */




	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(24)-6;*/				/* shuffle level */
	shamblerr->mmove = rn2(18)+9;				/* slow to very fast */
	shamblerr->ac = rn2(35)-24;				/* any AC */
	shamblerr->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerr->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(6); i++) {
		attkptr = &shamblerr->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(20)+2;				/* either too high or too low */
	}
	shamblerr->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerr->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerr->cnutrit = 20;					/* see above */
	shamblerr->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerr->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerr->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerr->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerr->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerr->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerr->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerr->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerr->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerr->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerr->mflags2 |= (1 << rn2(31));
	}
	shamblerr->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerr->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerr->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerr->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* second one */
	/*shambler->mlevel += rnd(24)-6;*/				/* shuffle level */
	shamblerrp->mmove = rn2(18)+9;				/* slow to very fast */
	shamblerrp->ac = rn2(35)-24;				/* any AC */
	shamblerrp->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerrp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(6); i++) {
		attkptr = &shamblerrp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(20)+2;				/* either too high or too low */
	}
	shamblerrp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerrp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerrp->cnutrit = 20;					/* see above */
	shamblerrp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerrp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerrp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerrp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerrp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerrp->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerrp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerrp->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerrp->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerrp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerrp->mflags2 |= (1 << rn2(31));
	}
	shamblerrp->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerrp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerrp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerrp->mflags2 &= ~M2_PNAME;				/* not a proper name */


	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblert->mmove = rn2(14)+9;				/* slow to very fast */
	shamblert->ac = rn2(31)-20;				/* any AC */
	shamblert->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblert->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblert->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblert->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblert->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblert->cnutrit = 20;					/* see above */
	shamblert->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblert->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblert->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblert->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblert->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblert->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblert->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	shamblert->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblert->mflags2 |= (1 << rn2(31));
	}
	shamblert->mflags2 &= ~M2_MERC;				/* no guards */
	shamblert->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblert->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblert->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblery->mmove = rn2(14)+9;				/* slow to very fast */
	shamblery->ac = rn2(31)-20;				/* any AC */
	shamblery->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblery->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblery->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblery->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblery->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblery->cnutrit = 20;					/* see above */
	shamblery->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblery->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblery->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblery->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblery->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblery->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblery->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	shamblery->mflags1 &= ~M1_HIDE;				/* no hiding */

	shamblery->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblery->mflags2 |= (1 << rn2(31));
	}
	shamblery->mflags2 &= ~M2_MERC;				/* no guards */
	shamblery->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblery->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblery->mflags2 &= ~M2_PNAME;				/* not a proper name */



	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblertp->mmove = rn2(14)+9;				/* slow to very fast */
	shamblertp->ac = rn2(31)-20;				/* any AC */
	shamblertp->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblertp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblertp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblertp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblertp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblertp->cnutrit = 20;					/* see above */
	shamblertp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblertp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblertp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblertp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblertp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblertp->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblertp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	shamblertp->mflags1 &= ~M1_CARNIVORE;				/* must be herbivore */

	shamblertp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblertp->mflags2 |= (1 << rn2(31));
	}
	shamblertp->mflags2 &= ~M2_MERC;				/* no guards */
	shamblertp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblertp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblertp->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	pokshamblert->mmove = rn2(14)+9;				/* slow to very fast */
	pokshamblert->ac = rn2(31)-20;				/* any AC */
	pokshamblert->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblert->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &pokshamblert->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	pokshamblert->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblert->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblert->cnutrit = 20;					/* see above */
	pokshamblert->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblert->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblert->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblert->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblert->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblert->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		pokshamblert->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	pokshamblert->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblert->mflags2 |= (1 << rn2(31));
	}
	pokshamblert->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblert->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblert->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblert->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	pokshamblertp->mmove = rn2(14)+9;				/* slow to very fast */
	pokshamblertp->ac = rn2(31)-20;				/* any AC */
	pokshamblertp->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblertp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &pokshamblertp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	pokshamblertp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblertp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblertp->cnutrit = 20;					/* see above */
	pokshamblertp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblertp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblertp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblertp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblertp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblertp->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		pokshamblertp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	pokshamblertp->mflags1 &= ~M1_CARNIVORE;				/* must be herbivore */

	pokshamblertp->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblertp->mflags2 |= (1 << rn2(31));
	}
	pokshamblertp->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblertp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblertp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblertp->mflags2 &= ~M2_PNAME;				/* not a proper name */


	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblerta->mmove = rn2(14)+9;				/* slow to very fast */
	shamblerta->ac = rn2(31)-20;				/* any AC */
	shamblerta->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerta->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblerta->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblerta->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerta->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerta->cnutrit = 20;					/* see above */
	shamblerta->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerta->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerta->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerta->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerta->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerta->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblerta->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	shamblerta->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerta->mflags2 |= (1 << rn2(31));
	}
	shamblerta->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerta->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerta->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerta->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblerya->mmove = rn2(14)+9;				/* slow to very fast */
	shamblerya->ac = rn2(31)-20;				/* any AC */
	shamblerya->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerya->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblerya->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblerya->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerya->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerya->cnutrit = 20;					/* see above */
	shamblerya->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerya->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerya->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerya->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerya->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerya->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblerya->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	shamblerya->mflags1 &= ~M1_HIDE;				/* no hiding */

	shamblerya->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerya->mflags2 |= (1 << rn2(31));
	}
	shamblerya->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerya->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerya->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerya->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblertpa->mmove = rn2(14)+9;				/* slow to very fast */
	shamblertpa->ac = rn2(31)-20;				/* any AC */
	shamblertpa->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblertpa->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblertpa->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblertpa->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblertpa->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblertpa->cnutrit = 20;					/* see above */
	shamblertpa->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblertpa->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblertpa->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblertpa->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblertpa->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblertpa->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblertpa->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	shamblertpa->mflags1 &= ~M1_CARNIVORE;				/* must be herbivore */

	shamblertpa->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblertpa->mflags2 |= (1 << rn2(31));
	}
	shamblertpa->mflags2 &= ~M2_MERC;				/* no guards */
	shamblertpa->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblertpa->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblertpa->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(12)-9;*/				/* shuffle level */
	shamblerl->mmove = rn2(8)+9;				/* slow to very fast */
	shamblerl->ac = rn2(16)-5;				/* any AC */
	shamblerl->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerl->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(3); i++) {
		attkptr = &shamblerl->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(5)+2;				/* either too high or too low */
	}
	shamblerl->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerl->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerl->cnutrit = 20;					/* see above */
	shamblerl->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerl->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerl->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerl->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerl->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerl->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerl->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerl->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerl->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerl->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerl->mflags2 |= (1 << rn2(31));
	}
	shamblerl->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerl->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerl->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerl->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(12)-9;*/				/* shuffle level */
	shamblerla->mmove = rn2(8)+9;				/* slow to very fast */
	shamblerla->ac = rn2(16)-5;				/* any AC */
	shamblerla->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerla->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(3); i++) {
		attkptr = &shamblerla->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(5)+2;				/* either too high or too low */
	}
	shamblerla->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerla->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerla->cnutrit = 20;					/* see above */
	shamblerla->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerla->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerla->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerla->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerla->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerla->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerla->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerla->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerla->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerla->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerla->mflags2 |= (1 << rn2(31));
	}
	shamblerla->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerla->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerla->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerla->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(12)-9;*/				/* shuffle level */
	pokshamblerl->mmove = rn2(8)+9;				/* slow to very fast */
	pokshamblerl->ac = rn2(16)-5;				/* any AC */
	pokshamblerl->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerl->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(3); i++) {
		attkptr = &pokshamblerl->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(5)+2;				/* either too high or too low */
	}
	pokshamblerl->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerl->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerl->cnutrit = 20;					/* see above */
	pokshamblerl->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerl->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerl->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerl->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerl->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerl->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerl->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerl->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerl->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerl->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerl->mflags2 |= (1 << rn2(31));
	}
	pokshamblerl->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerl->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerl->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerl->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(12)-9;*/				/* shuffle level */
	pokshamblerla->mmove = rn2(8)+9;				/* slow to very fast */
	pokshamblerla->ac = rn2(16)-5;				/* any AC */
	pokshamblerla->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerla->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(3); i++) {
		attkptr = &pokshamblerla->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(5)+2;				/* either too high or too low */
	}
	pokshamblerla->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerla->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerla->cnutrit = 20;					/* see above */
	pokshamblerla->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerla->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerla->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerla->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerla->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerla->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerla->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerla->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerla->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerla->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerla->mflags2 |= (1 << rn2(31));
	}
	pokshamblerla->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerla->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerla->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerla->mflags2 &= ~M2_PNAME;				/* not a proper name */



	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(16)-7;*/				/* shuffle level */
	shamblerx->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerx->ac = rn2(18)-7;				/* any AC */
	shamblerx->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerx->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(3); i++) {
		attkptr = &shamblerx->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(7)+2;				/* either too high or too low */
	}
	shamblerx->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerx->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerx->cnutrit = 20;					/* see above */
	shamblerx->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerx->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerx->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerx->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerx->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerx->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerx->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerx->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerx->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerx->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerx->mflags2 |= (1 << rn2(31));
	}
	shamblerx->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerx->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerx->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerx->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(16)-7;*/				/* shuffle level */
	shamblerxa->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerxa->ac = rn2(18)-7;				/* any AC */
	shamblerxa->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerxa->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(3); i++) {
		attkptr = &shamblerxa->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(7)+2;				/* either too high or too low */
	}
	shamblerxa->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerxa->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerxa->cnutrit = 20;					/* see above */
	shamblerxa->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerxa->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerxa->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerxa->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerxa->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerxa->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerxa->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerxa->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerxa->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerxa->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerxa->mflags2 |= (1 << rn2(31));
	}
	shamblerxa->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerxa->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerxa->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerxa->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(16)-7;*/				/* shuffle level */
	pokshamblerx->mmove = rn2(9)+9;				/* slow to very fast */
	pokshamblerx->ac = rn2(18)-7;				/* any AC */
	pokshamblerx->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerx->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(3); i++) {
		attkptr = &pokshamblerx->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(7)+2;				/* either too high or too low */
	}
	pokshamblerx->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerx->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerx->cnutrit = 20;					/* see above */
	pokshamblerx->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerx->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerx->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerx->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerx->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerx->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerx->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerx->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerx->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerx->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerx->mflags2 |= (1 << rn2(31));
	}
	pokshamblerx->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerx->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerx->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerx->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(16)-7;*/				/* shuffle level */
	pokshamblerxa->mmove = rn2(9)+9;				/* slow to very fast */
	pokshamblerxa->ac = rn2(18)-7;				/* any AC */
	pokshamblerxa->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerxa->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(3); i++) {
		attkptr = &pokshamblerxa->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(7)+2;				/* either too high or too low */
	}
	pokshamblerxa->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerxa->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerxa->cnutrit = 20;					/* see above */
	pokshamblerxa->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerxa->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerxa->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerxa->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerxa->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerxa->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerxa->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerxa->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerxa->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerxa->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerxa->mflags2 |= (1 << rn2(31));
	}
	pokshamblerxa->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerxa->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerxa->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerxa->mflags2 &= ~M2_PNAME;				/* not a proper name */


	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	shamblerm->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerm->ac = rn2(21)-10;				/* any AC */
	shamblerm->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerm->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerm->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	shamblerm->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerm->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerm->cnutrit = 20;					/* see above */
	shamblerm->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerm->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerm->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerm->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerm->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerm->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerm->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerm->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerm->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerm->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerm->mflags2 |= (1 << rn2(31));
	}
	shamblerm->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerm->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerm->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerm->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	shamblerma->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerma->ac = rn2(21)-10;				/* any AC */
	shamblerma->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerma->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerma->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	shamblerma->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerma->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerma->cnutrit = 20;					/* see above */
	shamblerma->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerma->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerma->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerma->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerma->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerma->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerma->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerma->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerma->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerma->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerma->mflags2 |= (1 << rn2(31));
	}
	shamblerma->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerma->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerma->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerma->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	shamblern->mmove = rn2(9)+9;				/* slow to very fast */
	shamblern->ac = rn2(21)-10;				/* any AC */
	shamblern->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblern->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblern->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	shamblern->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblern->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblern->cnutrit = 20;					/* see above */
	shamblern->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblern->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblern->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblern->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblern->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblern->mflags1 = M1_HIDE;
	for (i = 0; i < rnd(17); i++) {
		shamblern->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblern->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblern->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblern->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblern->mflags2 |= (1 << rn2(31));
	}
	shamblern->mflags2 &= ~M2_MERC;				/* no guards */
	shamblern->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblern->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblern->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	shamblerna->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerna->ac = rn2(21)-10;				/* any AC */
	shamblerna->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerna->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerna->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	shamblerna->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerna->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerna->cnutrit = 20;					/* see above */
	shamblerna->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerna->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerna->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerna->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerna->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerna->mflags1 = M1_HIDE;
	for (i = 0; i < rnd(17); i++) {
		shamblerna->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerna->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerna->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerna->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shamblerna->mflags2 |= (1 << rn2(31));
	}
	shamblerna->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerna->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerna->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerna->mflags2 &= ~M2_PNAME;				/* not a proper name */



	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	pokshamblerm->mmove = rn2(9)+9;				/* slow to very fast */
	pokshamblerm->ac = rn2(21)-10;				/* any AC */
	pokshamblerm->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerm->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &pokshamblerm->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	pokshamblerm->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerm->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerm->cnutrit = 20;					/* see above */
	pokshamblerm->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerm->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerm->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerm->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerm->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerm->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerm->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerm->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerm->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerm->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerm->mflags2 |= (1 << rn2(31));
	}
	pokshamblerm->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerm->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerm->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerm->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*pokshambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	pokshamblerma->mmove = rn2(9)+9;				/* slow to very fast */
	pokshamblerma->ac = rn2(21)-10;				/* any AC */
	pokshamblerma->mr = rn2(5)*25;				/* varying amounts of MR */
	pokshamblerma->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &pokshamblerma->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	pokshamblerma->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	pokshamblerma->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	pokshamblerma->cnutrit = 20;					/* see above */
	pokshamblerma->msound = rn2(MS_HUMANOID);			/* any but the specials */
	pokshamblerma->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		pokshamblerma->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		pokshamblerma->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	pokshamblerma->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	pokshamblerma->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		pokshamblerma->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*pokshamblerma->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*pokshamblerma->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	pokshamblerma->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		pokshamblerma->mflags2 |= (1 << rn2(31));
	}
	pokshamblerma->mflags2 &= ~M2_MERC;				/* no guards */
	pokshamblerma->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	pokshamblerma->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	pokshamblerma->mflags2 &= ~M2_PNAME;				/* not a proper name */

	/* what a horrible night to have a curse */
	/*shamblerplayer->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerplayer->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerplayer->ac = rn2(21)-10;				/* any AC */
	shamblerplayer->mr = rn2(11)*10;				/* varying amounts of MR */
	shamblerplayer->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblerplayer->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+1;				/* either too high or too low */
	}
	shamblerplayer->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayer->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayer->cnutrit = 20;					/* see above */
	shamblerplayer->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayer->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayer->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayer->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayer->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayer->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayer->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	shamblerplayer->mflags2 = M2_HOSTILE | M2_WERE;		/* mondata.c prevents players from polying into them */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayer->mflags2 |= (1 << rn2(31));
	}
	shamblerplayer->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayer->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayer->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayer->mflags2 &= ~M2_NOPOLY;				/* see above */


	/* what a horrible night to have a curse */
	/*shamblerplayerb->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerplayerb->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerplayerb->ac = rn2(21)-10;				/* any AC */
	shamblerplayerb->mr = rn2(11)*10;				/* varying amounts of MR */
	shamblerplayerb->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblerplayerb->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+1;				/* either too high or too low */
	}
	shamblerplayerb->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerb->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerb->cnutrit = 20;					/* see above */
	shamblerplayerb->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerb->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerb->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerb->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerb->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerb->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerb->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	shamblerplayerb->mflags2 = M2_HOSTILE | M2_WERE;		/* mondata.c prevents players from polying into them */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerb->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerb->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerb->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerb->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerb->mflags2 &= ~M2_NOPOLY;				/* see above */

	/* what a horrible night to have a curse */
	/*shamblerplayerc->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerplayerc->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerplayerc->ac = rn2(21)-10;				/* any AC */
	shamblerplayerc->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerc->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerplayerc->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblerplayerc->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerc->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerc->cnutrit = 20;					/* see above */
	shamblerplayerc->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerc->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerc->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerc->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerc->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerc->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerc->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayerc->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayerc->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayerc->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerc->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerc->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerc->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerc->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerc->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerc->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shamblerplayerd->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerplayerd->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerplayerd->ac = rn2(21)-10;				/* any AC */
	shamblerplayerd->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerd->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerplayerd->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblerplayerd->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerd->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerd->cnutrit = 20;					/* see above */
	shamblerplayerd->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerd->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerd->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerd->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerd->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerd->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerd->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayerd->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayerd->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayerd->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerd->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerd->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerd->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerd->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerd->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerd->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shamblerplayere->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerplayere->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerplayere->ac = rn2(21)-10;				/* any AC */
	shamblerplayere->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayere->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerplayere->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblerplayere->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayere->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayere->cnutrit = 20;					/* see above */
	shamblerplayere->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayere->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayere->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayere->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayere->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayere->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayere->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayere->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayere->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayere->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayere->mflags2 |= (1 << rn2(31));
	}
	shamblerplayere->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayere->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayere->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayere->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayere->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shamblerplayerf->mlevel += rnd(12)-3;*/				/* shuffle level */
	shamblerplayerf->mmove = rn2(10)+9;				/* slow to very fast */
	shamblerplayerf->ac = rn2(21)-10;				/* any AC */
	shamblerplayerf->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerf->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerplayerf->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	shamblerplayerf->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerf->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerf->cnutrit = 20;					/* see above */
	shamblerplayerf->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerf->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerf->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerf->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerf->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerf->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerf->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayerf->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayerf->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayerf->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerf->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerf->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerf->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerf->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerf->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerf->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	shamblerplayerg->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerplayerg->ac = rn2(21)-10;				/* any AC */
	shamblerplayerg->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerg->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerplayerg->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	shamblerplayerg->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerg->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerg->cnutrit = 20;					/* see above */
	shamblerplayerg->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerg->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerg->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerg->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerg->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerg->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerg->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayerg->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayerg->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayerg->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerg->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerg->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerg->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerg->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerg->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerg->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	shamblerplayerh->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerplayerh->ac = rn2(21)-10;				/* any AC */
	shamblerplayerh->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerh->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerplayerh->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	shamblerplayerh->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerh->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerh->cnutrit = 20;					/* see above */
	shamblerplayerh->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerh->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerh->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerh->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerh->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerh->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerh->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayerh->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayerh->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayerh->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerh->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerh->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerh->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerh->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerh->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerh->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	shamblerplayeri->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerplayeri->ac = rn2(21)-10;				/* any AC */
	shamblerplayeri->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayeri->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerplayeri->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	shamblerplayeri->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayeri->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayeri->cnutrit = 20;					/* see above */
	shamblerplayeri->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayeri->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayeri->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayeri->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayeri->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayeri->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayeri->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayeri->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayeri->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayeri->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayeri->mflags2 |= (1 << rn2(31));
	}
	shamblerplayeri->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayeri->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayeri->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayeri->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayeri->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(15)-9;*/				/* shuffle level */
	shamblerplayerj->mmove = rn2(9)+9;				/* slow to very fast */
	shamblerplayerj->ac = rn2(21)-10;				/* any AC */
	shamblerplayerj->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerj->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shamblerplayerj->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(6)+2;				/* either too high or too low */
	}
	shamblerplayerj->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerj->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerj->cnutrit = 20;					/* see above */
	shamblerplayerj->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerj->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerj->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerj->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerj->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerj->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerj->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayerj->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayerj->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayerj->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerj->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerj->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerj->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerj->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerj->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerj->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblerplayerk->mmove = rn2(14)+9;				/* slow to very fast */
	shamblerplayerk->ac = rn2(31)-20;				/* any AC */
	shamblerplayerk->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerk->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblerplayerk->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblerplayerk->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerk->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerk->cnutrit = 20;					/* see above */
	shamblerplayerk->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerk->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerk->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerk->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerk->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerk->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerk->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	shamblerplayerk->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerk->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerk->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerk->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerk->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerk->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerk->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblerplayerl->mmove = rn2(14)+9;				/* slow to very fast */
	shamblerplayerl->ac = rn2(31)-20;				/* any AC */
	shamblerplayerl->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerl->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblerplayerl->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblerplayerl->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerl->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerl->cnutrit = 20;					/* see above */
	shamblerplayerl->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerl->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerl->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerl->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerl->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerl->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerl->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	shamblerplayerl->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerl->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerl->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerl->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerl->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerl->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerl->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblerplayerm->mmove = rn2(14)+9;				/* slow to very fast */
	shamblerplayerm->ac = rn2(31)-20;				/* any AC */
	shamblerplayerm->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerm->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblerplayerm->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblerplayerm->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerm->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerm->cnutrit = 20;					/* see above */
	shamblerplayerm->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerm->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerm->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerm->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerm->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerm->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerm->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	shamblerplayerm->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerm->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerm->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerm->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerm->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerm->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerm->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(18)-4;*/				/* shuffle level */
	shamblerplayern->mmove = rn2(14)+9;				/* slow to very fast */
	shamblerplayern->ac = rn2(31)-20;				/* any AC */
	shamblerplayern->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayern->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &shamblerplayern->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	shamblerplayern->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayern->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayern->cnutrit = 20;					/* see above */
	shamblerplayern->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayern->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayern->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayern->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayern->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayern->mflags1 = M1_HERBIVORE;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayern->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	shamblerplayern->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayern->mflags2 |= (1 << rn2(31));
	}
	shamblerplayern->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayern->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayern->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayern->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayern->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*shambler->mlevel += rnd(24)-6;*/				/* shuffle level */
	shamblerplayero->mmove = rn2(18)+9;				/* slow to very fast */
	shamblerplayero->ac = rn2(35)-24;				/* any AC */
	shamblerplayero->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayero->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(6); i++) {
		attkptr = &shamblerplayero->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(20)+2;				/* either too high or too low */
	}
	shamblerplayero->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayero->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayero->cnutrit = 20;					/* see above */
	shamblerplayero->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayero->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayero->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayero->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayero->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayero->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayero->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayero->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayero->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayero->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayero->mflags2 |= (1 << rn2(31));
	}
	shamblerplayero->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayero->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayero->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayero->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayero->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */

	/* what a horrible night to have a curse */
	/*pokshamblerxt->mlevel += rnd(24)-6;*/				/* shuffle level */
	shamblerplayerp->mmove = rn2(20)+9;				/* slow to very fast */
	shamblerplayerp->ac = rn2(40)-29;				/* any AC */
	shamblerplayerp->mr = rn2(5)*25;				/* varying amounts of MR */
	shamblerplayerp->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < (rnd(5) + 1); i++) {
		attkptr = &shamblerplayerp->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(24)+2;				/* either too high or too low */
	}
	shamblerplayerp->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shamblerplayerp->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shamblerplayerp->cnutrit = 20;					/* see above */
	shamblerplayerp->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shamblerplayerp->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shamblerplayerp->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shamblerplayerp->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shamblerplayerp->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shamblerplayerp->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerp->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	/*shamblerplayerp->mflags1 &= ~M1_UNSOLID;*/			/* no ghosts */
	/*shamblerplayerp->mflags1 &= ~M1_WALLWALK;*/			/* no wall-walkers */

	shamblerplayerp->mflags2 = M2_HOSTILE;		/* see mondata.h */
	for (i = 0; i < rnd(17); i++) {
		shamblerplayerp->mflags2 |= (1 << rn2(31));
	}
	shamblerplayerp->mflags2 &= ~M2_MERC;				/* no guards */
	shamblerplayerp->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shamblerplayerp->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shamblerplayerp->mflags2 &= ~M2_PNAME;				/* not a proper name */
	shamblerplayerp->mflags2 &= ~M2_NOPOLY;				/* polymorph ok for monsters */





	if (!rn2(10)) {
	attkptr = &deathraylord->mattk[4]; /* Yeenoghu gets finger of death */
	attkptr->aatyp = AT_MAGC; /* well, at least one out of ten games he does */
	attkptr->adtyp = AD_DISN; /* as a tribute to that fallthrough "bug" where he got a touch of death --Amy */
	attkptr->damn = 2;
	attkptr->damd = 6;
	}

	/* the multicolor grue has a AD_RBRE breath attack, but there is no AD_RBRE for melee attacks (yet).
	 * So we're just giving it some random damage types for its melee and passive attack. --Amy */
	for (i = 0; i < 2; i++) {
		attkptr = &multigrue->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	/* similar stuff for some other monsters */

	for (i = 0; i < 2; i++) {
		attkptr = &randomeye->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	randommimic->mmove = rnd(10)+2;
	randommimic->ac = 5-rnd(20);
	randommimic->mr = rn2(101);
	for (i = 0; i < 3; i++) {
		attkptr = &randommimic->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	for (i = 0; i < 2; i++) {
		attkptr = &randompiercer->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}
	for (i = 0; i < 2; i++) {
		attkptr = &randompiercerb->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}
	for (i = 0; i < 2; i++) {
		attkptr = &randompiercerc->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	randomvortex->mmove = rnd(10)+6;
	randomvortex->ac = 0-rnd(16);
	randomvortex->mr = rn2(101);
	for (i = 0; i < 2; i++) {
		attkptr = &randomvortex->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	randomfungus->ac = 10-rnd(25);
	randomfungus->mr = rn2(101);
	for (i = 0; i < 1; i++) {
		attkptr = &randomfungus->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	randomfungusb->mmove = rnd(12);
	randomfungusb->ac = 10-rnd(25);
	randomfungusb->mr = rn2(101);
	for (i = 0; i < 2; i++) {
		attkptr = &randomfungusb->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	randomfungusc->ac = 10-rnd(35);
	randomfungusc->mr = rn2(101);
	for (i = 0; i < 1; i++) {
		attkptr = &randomfungusc->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	randomfungusd->mmove = rnd(15);
	randomfungusd->ac = 10-rnd(35);
	randomfungusd->mr = rn2(101);
	for (i = 0; i < 2; i++) {
		attkptr = &randomfungusd->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	randomtroll->mmove = rnd(6)+10;
	randomtroll->ac = 0-rnd(20);
	randomtroll->mr = rn2(101);
	for (i = 0; i < 3; i++) {
		attkptr = &randomtroll->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	for (i = 1; i < 2; i++) {
		attkptr = &randomeel->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragon->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	for (i = 1; i < 2; i++) {
		attkptr = &shadowwarrior->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	for (i = 1; i < 2; i++) {
		attkptr = &shadowwarriorX->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragonb->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragonb->mcolor = rnd(15);
		if (randomdragonb->mcolor == 4) randomdragonb->mcolor = 12;

	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragonc->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragonc->mcolor = rnd(15);
		if (randomdragonc->mcolor == 4) randomdragonc->mcolor = 12;

	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragond->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragond->mcolor = rnd(15);
		if (randomdragond->mcolor == 4) randomdragond->mcolor = 12;

	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragone->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragone->mcolor = rnd(15);
		if (randomdragone->mcolor == 4) randomdragone->mcolor = 12;
	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragonf->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragonf->mcolor = rnd(15);
		if (randomdragonf->mcolor == 4) randomdragonf->mcolor = 12;
	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragong->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragong->mcolor = rnd(15);
		if (randomdragong->mcolor == 4) randomdragong->mcolor = 12;
	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragonh->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragonh->mcolor = rnd(15);
		if (randomdragonh->mcolor == 4) randomdragonh->mcolor = 12;
	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragoni->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragoni->mcolor = rnd(15);
		if (randomdragoni->mcolor == 4) randomdragoni->mcolor = 12;
	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragonj->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragonj->mcolor = rnd(15);
		if (randomdragonj->mcolor == 4) randomdragonj->mcolor = 12;
	}

	for (i = 0; i < 1; i++) {
		attkptr = &randomdragonk->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = rnd(9);
		}

		randomdragonk->mcolor = rnd(15);
		if (randomdragonk->mcolor == 4) randomdragonk->mcolor = 12;
	}

	randombdragonb->mcolor = randomdragonb->mcolor;
	randombdragonc->mcolor = randomdragonc->mcolor;
	randombdragond->mcolor = randomdragond->mcolor;
	randombdragone->mcolor = randomdragone->mcolor;
	randombdragonf->mcolor = randomdragonf->mcolor;
	randombdragong->mcolor = randomdragong->mcolor;
	randombdragonh->mcolor = randomdragonh->mcolor;
	randombdragoni->mcolor = randomdragoni->mcolor;
	randombdragonj->mcolor = randomdragonj->mcolor;
	randombdragonk->mcolor = randomdragonk->mcolor;

	for (i = 1; i < 2; i++) {
		attkptr = &randomflyfish->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	for (i = 0; i < 3; i++) {
		attkptr = &randomturret->mattk[i];

		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}

	}

	randomkop->mmove = rn2(10)+9;				/* slow to very fast */
	randomkop->ac = rn2(21)-10;				/* any AC */
	randomkop->mr = rn2(5)*25;				/* varying amounts of MR */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &randomkop->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(10)+2;				/* either too high or too low */
	}
	randomkop->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		randomkop->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		randomkop->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	randomkop->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	randomkop->mflags1 = M1_BREATHLESS|M1_MINDLESS|M1_HUMANOID;
	for (i = 0; i < rnd(17); i++) {
		randomkop->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	randomkop->mflags2 = M2_HUMAN|M2_STALK|M2_NOPOLY|M2_HOSTILE|M2_STRONG|M2_NEUTER|M2_NASTY;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		randomkop->mflags2 |= (1 << rn2(31));
	}
	randomkop->mflags2 &= ~M2_MERC;				/* no guards */
	randomkop->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	randomkop->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	randomkop->mflags2 &= ~M2_PNAME;				/* not a proper name */

	randomkopb->mmove = rn2(14)+9;				/* slow to very fast */
	randomkopb->ac = rn2(31)-20;				/* any AC */
	randomkopb->mr = rn2(5)*25;				/* varying amounts of MR */
	/* attacks...?  */
	for (i = 0; i < rnd(5); i++) {
		attkptr = &randomkopb->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(15)+2;				/* either too high or too low */
	}
	randomkopb->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		randomkopb->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		randomkopb->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	randomkopb->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	randomkopb->mflags1 = M1_BREATHLESS|M1_MINDLESS|M1_HUMANOID;
	for (i = 0; i < rnd(17); i++) {
		randomkopb->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	randomkopb->mflags2 = M2_HUMAN|M2_STALK|M2_NOPOLY|M2_HOSTILE|M2_STRONG|M2_NEUTER|M2_NASTY;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		randomkopb->mflags2 |= (1 << rn2(31));
	}
	randomkopb->mflags2 &= ~M2_MERC;				/* no guards */
	randomkopb->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	randomkopb->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	randomkopb->mflags2 &= ~M2_PNAME;				/* not a proper name */

	randomkopc->mmove = rn2(20)+9;				/* slow to very fast */
	randomkopc->ac = rn2(40)-29;				/* any AC */
	randomkopc->mr = rn2(5)*25;				/* varying amounts of MR */
	/* attacks...?  */
	for (i = 0; i < (rnd(5) + 1); i++) {
		attkptr = &randomkopc->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = AT_MULTIPLY;
		while (attkptr->aatyp == AT_MULTIPLY) {
			attkptr->aatyp = rn2(AT_MULTIPLY);
		}
		if (attkptr->aatyp == AT_BOOM) {
			attkptr->aatyp = AT_MAGC;
		}
		if (attkptr->aatyp == AT_EXPL) {
			attkptr->aatyp = AT_WEAP;
		}
		attkptr->adtyp = AD_ENDS;
		while (attkptr->adtyp == AD_ENDS || attkptr->adtyp == AD_SPC2 || attkptr->adtyp == AD_WERE) {
			attkptr->adtyp = randattack();
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = rnd(24)+2;				/* either too high or too low */
	}
	randomkopc->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		randomkopc->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		randomkopc->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	randomkopc->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	randomkopc->mflags1 = M1_BREATHLESS|M1_MINDLESS|M1_HUMANOID;
	for (i = 0; i < rnd(17); i++) {
		randomkopc->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}

	randomkopc->mflags2 = M2_HUMAN|M2_STALK|M2_NOPOLY|M2_HOSTILE|M2_STRONG|M2_NEUTER|M2_NASTY;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		randomkopc->mflags2 |= (1 << rn2(31));
	}
	randomkopc->mflags2 &= ~M2_MERC;				/* no guards */
	randomkopc->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	randomkopc->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	randomkopc->mflags2 &= ~M2_PNAME;				/* not a proper name */


	return;
}


/* skills aren't initialized, so we use the role-specific skill lists */
STATIC_OVL boolean
restricted_spell_discipline(otyp)
int otyp;
{
    const struct def_skill *skills;
    int this_skill = spell_skilltype(otyp);

    switch (Role_switch) {
     case PM_ARCHEOLOGIST:	skills = Skill_A; break;
     case PM_LIBRARIAN:	skills = Skill_Lib; break;
     case PM_SUPERMARKET_CASHIER:	skills = Skill_Sup; break;
     case PM_INTEL_SCRIBE:	skills = Skill_Scr; break;
     case PM_GAMER:	skills = Skill_Gam; break;
     case PM_ARTIST:	skills = Skill_Art; break;
     case PM_AUGURER:	skills = Skill_Aug; break;
     case PM_BARBARIAN:		skills = Skill_B; break;
     case PM_BARD:		skills = Skill_Bard; break;
     case PM_BINDER:			skills = Skill_Bin; break;
     case PM_POLITICIAN:			skills = Skill_Pol; break;
     case PM_BLEEDER:			skills = Skill_Ble; break;
     case PM_COOK:			skills = Skill_Coo; break;
     case PM_CAVEMAN:		skills = Skill_C; break;
#ifdef CONVICT
     case PM_CONVICT:		skills = Skill_Con; break;
#endif  /* CONVICT */
     case PM_COURIER:		skills = Skill_Cou; break;
     case PM_SPACEWARS_FIGHTER:		skills = Skill_Spa; break;
     case PM_ROCKER:		skills = Skill_Roc; break;
     case PM_ZYBORG:		skills = Skill_Zyb; break;
     case PM_LUNATIC:		skills = Skill_Lun; break;
     case PM_CHEVALIER:		skills = Skill_Che; break;
     case PM_ELECTRIC_MAGE:		skills = Skill_Ele; break;
     case PM_CRUEL_ABUSER:		skills = Skill_Abu; break;
     case PM_SLAVE_MASTER:		skills = Skill_Sla; break;
     case PM_MYSTIC:		skills = Skill_Mys; break;
     case PM_BLOODSEEKER:		skills = Skill_Blo; break;
     case PM_LADIESMAN:		skills = Skill_Lad; break;
     case PM_JESTER:		skills = Skill_Jes; break;
     case PM_ACID_MAGE:		skills = Skill_Aci; break;
     case PM_FLAME_MAGE:		skills = Skill_F; break;
     case PM_TRANSVESTITE:	skills = Skill_Tra; break;
     case PM_TOPMODEL:	skills = Skill_Top; break;
     case PM_GEEK:		skills = Skill_G; break;
     case PM_SCIENTIST:		skills = Skill_Sci; break;
     case PM_GANGSTER:		skills = Skill_Gan; break;
     case PM_GUNNER:		skills = Skill_Gun; break;
     case PM_HEALER:		skills = Skill_H; break;
     case PM_JEDI:		skills = Skill_J; break;
     case PM_KNIGHT:		skills = Skill_K; break;
     case PM_WANDKEEPER:		skills = Skill_Wan; break;
     case PM_PALADIN:		skills = Skill_Pal; break;
     case PM_FEAT_MASTER:		skills = Skill_Stu; break;
     case PM_FOXHOUND_AGENT:		skills = Skill_Fox; break;
     case PM_WARRIOR:		skills = Skill_War; break;
     case PM_DOLL_MISTRESS:		skills = Skill_Dol; break;
     case PM_MAHOU_SHOUJO:		skills = Skill_Mah; break;
     case PM_MONK:		skills = Skill_Mon; break;
     case PM_SAIYAN:		skills = Skill_Sai; break;
     case PM_PSION:		skills = Skill_Psi; break;
	 case PM_PIRATE:		skills = Skill_Pir; break;
	 case PM_KORSAIR:		skills = Skill_Kor; break;
	 case PM_GLADIATOR:		skills = Skill_Gla; break;
	 case PM_GOFF:		skills = Skill_Gof; break;
	 case PM_AMAZON:		skills = Skill_Ama; break;
	 case PM_ALTMER:		skills = Skill_Alt; break;
	 case PM_BOSMER:		skills = Skill_Bos; break;
	 case PM_DUNMER:		skills = Skill_Dun; break;
	 case PM_ORDINATOR:		skills = Skill_Ord; break;
	 case PM_THALMOR:		skills = Skill_Tha; break;
	 case PM_DIVER:		skills = Skill_Div; break;
     case PM_POKEMON:		skills = Skill_Pok; break;
     case PM_PRIEST:		skills = Skill_P; break;
     case PM_RANGER:		skills = Skill_Ran; break;
     case PM_ELPH:		skills = Skill_Elp; break;
     case PM_ROGUE:		skills = Skill_R; break;
     case PM_PICKPOCKET:		skills = Skill_Pic; break;
     case PM_BULLY:		skills = Skill_Bul; break;
     case PM_ASSASSIN:		skills = Skill_Ass; break;
     case PM_SAMURAI:		skills = Skill_S; break;
#ifdef TOURIST
     case PM_TOURIST:		skills = Skill_T; break;
#endif
     case PM_OTAKU:		skills = Skill_Ota; break;
     case PM_VALKYRIE:		skills = Skill_V; break;
     case PM_WIZARD:		skills = Skill_W; break;
     case PM_SAGE:		skills = Skill_Sag; break;

     case PM_DRUNK:		skills = Skill_Dru; break;
     case PM_MUSICIAN:		skills = Skill_Mus; break;
     case PM_LOCKSMITH:		skills = Skill_Loc; break;
     case PM_FIREFIGHTER:		skills = Skill_Fir; break;
     case PM_OFFICER:		skills = Skill_Off; break;
     case PM_ZOOKEEPER:		skills = Skill_Zoo; break;
     case PM_NINJA:		skills = Skill_Nin; break;
     case PM_UNDERTAKER:		skills = Skill_Unt; break;
     case PM_GRADUATE:		skills = Skill_Gra; break;

     default:			skills = 0; break;	/* lint suppression */
    }

    while (skills->skill != P_NONE) {
	if (skills->skill == this_skill) return FALSE;
	++skills;
    }
    return TRUE;
}

STATIC_OVL void
ini_inv(trop)
register struct trobj *trop;
{
	struct obj *obj;
	struct obj *objX;
	int otyp, i;

	while (trop->trclass) {
		if (trop->trotyp != UNDEF_TYP) {
			otyp = (int)trop->trotyp;
			if (urace.malenum != PM_HUMAN) {
			    /* substitute specific items for generic ones */
			    for (i = 0; inv_subs[i].race_pm != NON_PM; ++i)
				if (inv_subs[i].race_pm == urace.malenum &&
					otyp == inv_subs[i].item_otyp) {
				    otyp = inv_subs[i].subs_otyp;
				    break;
				}
			}
			for (i = 0; inv_asubs[i].align != A_NONE; ++i)
			    if (inv_asubs[i].align == u.ualign.type &&
				    otyp == inv_asubs[i].item_otyp) {
				otyp = inv_asubs[i].subs_otyp;
				break;
			    }
			obj = mksobj(otyp, TRUE, FALSE);
			if (Race_if(PM_HAXOR)) objX = mksobj(otyp, TRUE, FALSE);
		} else {	/* UNDEF_TYP */
			static NEARDATA short nocreate = STRANGE_OBJECT;
			static NEARDATA short nocreate2 = STRANGE_OBJECT;
			static NEARDATA short nocreate3 = STRANGE_OBJECT;
			static NEARDATA short nocreate4 = STRANGE_OBJECT;
		/*
		 * For random objects, do not create certain overly powerful
		 * items: wand of wishing, ring of levitation, or the
		 * polymorph/polymorph control combination.  Specific objects,
		 * i.e. the discovery wishing, are still OK.
		 * Also, don't get a couple of really useless items.  (Note:
		 * punishment isn't "useless".  Some players who start out with
		 * one will immediately read it and use the iron ball as a
		 * weapon.)
		 */
			obj = mkobj(trop->trclass, FALSE);
			if (Race_if(PM_HAXOR)) objX = mkobj(trop->trclass, FALSE);
			otyp = obj->otyp;
			while (otyp == WAN_WISHING
				|| otyp == WAN_ACQUIREMENT
				|| otyp == SCR_WISHING
				|| otyp == SCR_ACQUIREMENT
				|| otyp == SCR_ENTHRONIZATION
				|| otyp == SCR_FOUNTAIN_BUILDING
				|| otyp == SCR_SINKING
				|| otyp == SCR_WC
				|| otyp == nocreate
				|| otyp == nocreate2
				|| otyp == nocreate3
				|| otyp == nocreate4
#ifdef ELBERETH
				|| otyp == RIN_LEVITATION
#endif
	/*			|| ((Role_if(PM_FLAME_MAGE) || Role_if(PM_ICE_MAGE))
						&&
				    (otyp == RIN_FIRE_RESISTANCE || 
				     otyp == RIN_COLD_RESISTANCE ||
				     otyp == SPE_ENDURE_HEAT ||
				     otyp == SPE_ENDURE_COLD)) */

				/* KMH -- Hobbits shouldn't get ring of invis. */
				/* || (Role_if(PM_HOBBIT) && otyp == RIN_INVISIBILITY) */

				/* KMH, balance patch -- now an amulet */
				/*|| (Role_if(PM_NECROMANCER) &&
						otyp == AMULET_OF_DRAIN_RESISTANCE)*/
				/* 'useless' or over powerful items */
				/*|| otyp == POT_HALLUCINATION
				|| otyp == POT_ACID
				|| otyp == SCR_AMNESIA
				|| otyp == SCR_FIRE
				|| otyp == SCR_BLANK_PAPER
				|| otyp == SPE_BLANK_PAPER
				|| otyp == RIN_AGGRAVATE_MONSTER
				|| otyp == RIN_HUNGER
				|| otyp == RIN_SLEEPING
				|| otyp == WAN_NOTHING*/
				/* Monks don't use weapons */
				/*|| (otyp == SCR_ENCHANT_WEAPON &&
				    Role_if(PM_MONK))*/
				/* wizard patch -- they already have one */
				/*|| (otyp == SPE_FORCE_BOLT &&
				    Role_if(PM_WIZARD))*/
				/* powerful spells are either useless to
				   low level players or unbalancing; also
				   spells in restricted skill categories */
				/*|| (obj->oclass == SPBOOK_CLASS &&
				    (objects[otyp].oc_level > 3 ||
				    restricted_spell_discipline(otyp)))*/
							) {
				dealloc_obj(obj);
				obj = mkobj(trop->trclass, FALSE);
				otyp = obj->otyp;
			} /* re-enabled a lot of stuff. Startscum if you really have to. --Amy */

			if (Race_if(PM_HAXOR)) {

			if (!Hallucination) {

			obj->dknown = obj->bknown = obj->rknown = 1;
			if (objects[otyp].oc_uses_known) obj->known = 1;
			obj->cursed = 0;
			if (obj->opoisoned && u.ualign.type != A_CHAOTIC)
			    obj->opoisoned = 0;
			if (obj->oclass == WEAPON_CLASS ||
				obj->oclass == TOOL_CLASS) {
			    obj->quan = (long) trop->trquan;
			    trop->trquan = 1;
			} else if (obj->oclass == GEM_CLASS &&
				is_graystone(obj) && obj->otyp != FLINT) {
			    obj->quan = 1L;
			}
			if (OBJ_DESCR(objects[otyp]) && obj->known)
			discover_object(otyp, TRUE, FALSE);

			}

			otyp = objX->otyp;
			while (otyp == WAN_WISHING
				|| otyp == WAN_ACQUIREMENT
				|| otyp == SCR_WISHING
				|| otyp == SCR_ACQUIREMENT
				|| otyp == SCR_ENTHRONIZATION
				|| otyp == SCR_FOUNTAIN_BUILDING
				|| otyp == SCR_SINKING
				|| otyp == SCR_WC
				|| otyp == nocreate
				|| otyp == nocreate2
				|| otyp == nocreate3
				|| otyp == nocreate4
#ifdef ELBERETH
				|| otyp == RIN_LEVITATION
#endif
	/*			|| ((Role_if(PM_FLAME_MAGE) || Role_if(PM_ICE_MAGE))
						&&
				    (otyp == RIN_FIRE_RESISTANCE || 
				     otyp == RIN_COLD_RESISTANCE ||
				     otyp == SPE_ENDURE_HEAT ||
				     otyp == SPE_ENDURE_COLD)) */

				/* KMH -- Hobbits shouldn't get ring of invis. */
				/* || (Role_if(PM_HOBBIT) && otyp == RIN_INVISIBILITY) */

				/* KMH, balance patch -- now an amulet */
				/*|| (Role_if(PM_NECROMANCER) &&
						otyp == AMULET_OF_DRAIN_RESISTANCE)*/
				/* 'useless' or over powerful items */
				/*|| otyp == POT_HALLUCINATION
				|| otyp == POT_ACID
				|| otyp == SCR_AMNESIA
				|| otyp == SCR_FIRE
				|| otyp == SCR_BLANK_PAPER
				|| otyp == SPE_BLANK_PAPER
				|| otyp == RIN_AGGRAVATE_MONSTER
				|| otyp == RIN_HUNGER
				|| otyp == RIN_SLEEPING
				|| otyp == WAN_NOTHING*/
				/* Monks don't use weapons */
				/*|| (otyp == SCR_ENCHANT_WEAPON &&
				    Role_if(PM_MONK))*/
				/* wizard patch -- they already have one */
				/*|| (otyp == SPE_FORCE_BOLT &&
				    Role_if(PM_WIZARD))*/
				/* powerful spells are either useless to
				   low level players or unbalancing; also
				   spells in restricted skill categories */
				/*|| (objX->oclass == SPBOOK_CLASS &&
				    (objects[otyp].oc_level > 3 ||
				    restricted_spell_discipline(otyp)))*/
							) {
				dealloc_obj(objX);
				objX = mkobj(trop->trclass, FALSE);
				otyp = objX->otyp;
			}
			}
#ifdef JEDI
			if (is_lightsaber(obj))
				obj->age = 1500;
			if (Race_if(PM_HAXOR)) {
			if (is_lightsaber(objX))
				objX->age = 1500;
			}
			/* start with maxed lightsaber
			only Jedi start with one, so no need to check
			Role_if here */
#endif

			/* Don't start with +0 or negative rings */
			if (objects[otyp].oc_charged && obj->spe </*=*/ 0)
				obj->spe = rne(2);

			if ((obj->oclass == WEAPON_CLASS || obj->oclass == ARMOR_CLASS) && !rn2(3))
				obj->spe = rne(2);

			if (Race_if(PM_HAXOR)) {
			if ((objX->oclass == WEAPON_CLASS || objX->oclass == ARMOR_CLASS) && !rn2(3))
				objX->spe = rne(2);
			}

			/* Heavily relies on the fact that 1) we create wands
			 * before rings, 2) that we create rings before
			 * spellbooks, and that 3) not more than 1 object of a
			 * particular symbol is to be prohibited.  (For more
			 * objects, we need more nocreate variables...)
			 */
			/*switch (otyp) {
			    case WAN_POLYMORPH:
			    case RIN_POLYMORPH:
			    case POT_POLYMORPH:
				nocreate = RIN_POLYMORPH_CONTROL;
				break;
			    case RIN_POLYMORPH_CONTROL:
				nocreate = RIN_POLYMORPH;
				nocreate2 = SPE_POLYMORPH;
				nocreate3 = POT_POLYMORPH;
			}*/
			/* Don't have 2 of the same ring or spellbook */
			/*if (obj->oclass == RING_CLASS ||
			    obj->oclass == SPBOOK_CLASS)
				nocreate4 = otyp;*/
		}

#ifdef GOLDOBJ
		if (trop->trclass == COIN_CLASS) {
			/* no "blessed" or "identified" money */
			obj->quan = u.umoney0;
		} else {
#endif
			obj->dknown = obj->bknown = obj->rknown = 1;
			if (objects[otyp].oc_uses_known) obj->known = 1;
			obj->cursed = 0;
			if (obj->opoisoned && u.ualign.type != A_CHAOTIC)
			    obj->opoisoned = 0;
			if (obj->oclass == WEAPON_CLASS ||
				obj->oclass == TOOL_CLASS) {
			    obj->quan = (long) trop->trquan;
			    trop->trquan = 1;
			} else if (obj->oclass == GEM_CLASS &&
				is_graystone(obj) && obj->otyp != FLINT) {
			    obj->quan = 1L;
			}


		if (Race_if(PM_HAXOR)) {


			objX->dknown = objX->bknown = objX->rknown = 1;
			if (objects[otyp].oc_uses_known) objX->known = 1;
			objX->cursed = 0;
			if (objX->opoisoned && u.ualign.type != A_CHAOTIC)
			    objX->opoisoned = 0;
			if (objX->oclass == WEAPON_CLASS ||
				objX->oclass == TOOL_CLASS) {
			    objX->quan = (long) trop->trquan;
			    trop->trquan = 1;
			} else if (objX->oclass == GEM_CLASS &&
				is_graystone(objX) && objX->otyp != FLINT) {
			    objX->quan = 1L;
			}

#ifdef CONVICT
            if (objX->otyp == STRIPED_SHIRT ) {
                objX->cursed = TRUE;
            }
#endif /* CONVICT */
            if (objX->otyp == VICTORIAN_UNDERWEAR ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == T_SHIRT ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == HAWAIIAN_SHIRT ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == RUFFLED_SHIRT ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == LOADSTONE ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == WAN_DEATH && (Role_if(PM_DEATH_EATER) || Race_if(PM_UNGENOMOLD) ) ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == WAN_TELEPORTATION && (Race_if(PM_MAIA) ) ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == LUMP_OF_ROYAL_JELLY && (Race_if(PM_INSECTOID) ) ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == AMULET_OF_LIFE_SAVING && (Role_if(PM_DEATH_EATER)) ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == AMULET_OF_COVETOUS_WARNING && (Role_if(PM_GOFF)) ) {
                objX->cursed = TRUE;
            }
            if (objX->otyp == BLACK_DRAGON_SCALE_MAIL && (Role_if(PM_GOFF)) ) {
                objX->cursed = TRUE;
            }
			if (trop->trspe != UNDEF_SPE)
			    objX->spe = trop->trspe;
			if (trop->trbless != UNDEF_BLESS)
			    objX->blessed = trop->trbless;
		}



#ifdef CONVICT
            if (obj->otyp == STRIPED_SHIRT ) {
                obj->cursed = TRUE;
            }
#endif /* CONVICT */
            if (obj->otyp == VICTORIAN_UNDERWEAR ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == T_SHIRT ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == HAWAIIAN_SHIRT ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == RUFFLED_SHIRT ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == LOADSTONE ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == WAN_DEATH && (Role_if(PM_DEATH_EATER) || Race_if(PM_UNGENOMOLD) ) ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == WAN_TELEPORTATION && (Race_if(PM_MAIA) ) ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == LUMP_OF_ROYAL_JELLY && (Race_if(PM_INSECTOID) ) ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == ASIAN_PEAR && (Role_if(PM_FOXHOUND_AGENT) ) ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == AMULET_OF_LIFE_SAVING && (Role_if(PM_DEATH_EATER)) ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == AMULET_OF_COVETOUS_WARNING && (Role_if(PM_GOFF)) ) {
                obj->cursed = TRUE;
            }
            if (obj->otyp == BLACK_DRAGON_SCALE_MAIL && (Role_if(PM_GOFF)) ) {
                obj->cursed = TRUE;
            }
			if (trop->trspe != UNDEF_SPE)
			    obj->spe = trop->trspe;
			if (trop->trbless != UNDEF_BLESS)
			    obj->blessed = trop->trbless;
#ifdef GOLDOBJ
		}
#endif
		/* defined after setting otyp+quan + blessedness */
		obj->owt = weight(obj);
				
		obj = addinv(obj);
		if (Race_if(PM_HAXOR)) objX = addinv(objX);

		/* Make the type known if necessary */
		if (OBJ_DESCR(objects[otyp]) && obj->known)
			discover_object(otyp, TRUE, FALSE);

		if (Race_if(PM_HAXOR)) {
		if (OBJ_DESCR(objects[otyp]) && objX->known)
			discover_object(otyp, TRUE, FALSE);
		}

		if (otyp == OIL_LAMP)
			discover_object(POT_OIL, TRUE, FALSE);

		if(obj->oclass == ARMOR_CLASS){
			if (is_shield(obj) && !uarms) {
				setworn(obj, W_ARMS);
				if (uswapwep) 
				  setuswapwep((struct obj *) 0, TRUE);
			} else if (is_helmet(obj) && !uarmh)
				setworn(obj, W_ARMH);
			else if (is_gloves(obj) && !uarmg)
				setworn(obj, W_ARMG);
#ifdef TOURIST
			else if (is_shirt(obj) && !uarmu)
				setworn(obj, W_ARMU);
#endif
			else if (is_cloak(obj) && !uarmc)
				setworn(obj, W_ARMC);
			else if (is_boots(obj) && !uarmf)
				setworn(obj, W_ARMF);
			else if (is_suit(obj) && !uarm)
				setworn(obj, W_ARM);
		}

		if(obj->otyp == AMULET_OF_LIFE_SAVING && (Role_if(PM_DEATH_EATER)) ) {
				setworn(obj, W_AMUL);
			}

            if (obj->otyp == AMULET_OF_COVETOUS_WARNING && (Role_if(PM_GOFF)) ) {
				setworn(obj, W_AMUL);
            }


		if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
			otyp == TIN_OPENER || otyp == FLINT || otyp == ROCK) {
		    if (is_ammo(obj) || is_missile(obj)) {
			if (!uquiver) setuqwep(obj);
		    } else if (!uwep) setuwep(obj, FALSE);
		    else if (!uswapwep) setuswapwep(obj, FALSE);
		}
		if (obj->oclass == SPBOOK_CLASS &&
				obj->otyp != SPE_BLANK_PAPER)
		    initialspell(obj);

#if !defined(PYRAMID_BUG) && !defined(MAC)
		if(--trop->trquan) continue;	/* make a similar object */
#else
		if(trop->trquan) {		/* check if zero first */
			--trop->trquan;
			if(trop->trquan)
				continue;	/* make a similar object */
		}
#endif
		
		trop++;
	}

	/*if (!strncmpi(plname, "lostsoul", 8)) { 
	goto_level(&medusa_level, FALSE, FALSE, FALSE); inspired by Tome, an Angband mod --Amy
	}*/

}

/*u_init.c*/
