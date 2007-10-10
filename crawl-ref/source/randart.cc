/*
 *  File:       randart.cc
 *  Summary:    Random and unrandom artefact functions.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 *   <8>     19 Jun 99   GDL    added IBMCPP support
 *   <7>     14/12/99    LRH    random2 -> random5
 *   <6>     11/06/99    cdl    random4 -> random2
 *
 *   <1>     -/--/--     LRH    Created
 */

#include "AppHdr.h"
#include "randart.h"

#include <cstdlib>
#include <climits>
#include <string.h>
#include <stdio.h>

#include "externs.h"
#include "itemname.h"
#include "itemprop.h"
#include "stuff.h"

#define KNOWN_PROPS_KEY "randart_known_props"

/*
   The initial generation of a randart is very simple - it occurs
   in dungeon.cc and consists of giving it a few random things - plus & plus2
   mainly.
*/
static const char *rand_wpn_names[] = {
    " of Blood",
    " of Death",
    " of Bloody Death",
    " of Pain",
    " of Painful Death",
    " of Pain & Death",
    " of Infinite Pain",
    " of Eternal Torment",
    " of Power",
    " of Wrath",
/* 10: */
    " of Doom",
    " of Tender Mercy",
    " of the Apocalypse",
    " of the Jester",
    " of the Ring",
    " of the Fool",
    " of the Gods",
    " of the Imperium",
    " of Destruction",
    " of Armageddon",
/* 20: */
    " of Cruel Justice",
    " of Righteous Anger",
    " of Might",
    " of the Orb",
    " of Makhleb",
    " of Trog",
    " of Xom",
    " of the Ancients",
    " of Mana",
    " of Nemelex Xobeh",
/* 30: */
    " of the Magi",
    " of the Archmagi",
    " of the King",
    " of the Queen",
    " of the Spheres",
    " of Circularity",
    " of Linearity",
    " of Conflict",
    " of Battle",
    " of Honour",
/* 40: */
    " of the Butterfly",
    " of the Wasp",
    " of the Frog",
    " of the Weasel",
    " of the Troglodytes",
    " of the Pill-Bug",
    " of Sin",
    " of Vengeance",
    " of Execution",
    " of Arbitration",
/* 50: */
    " of the Seeker",
    " of Truth",
    " of Lies",
    " of the Eggplant",
    " of the Turnip",
    " of Chance",
    " of Curses",
    " of Hell's Wrath",
    " of the Undead",
    " of Chaos",
/* 60: */
    " of Law",
    " of Life",
    " of the Old World",
    " of the New World",
    " of the Middle World",
    " of Crawl",
    " of Unpleasantness",
    " of Discomfort",
    " of Brutal Revenge",
    " of Triumph",
/* 70: */
    " of Evisceration",
    " of Dismemberment",
    " of Terror",
    " of Fear",
    " of Pride",
    " of the Volcano",
    " of Blood-Lust",
    " of Division",
    " of Eternal Harmony",
    " of Peace",
/* 80: */
    " of Quick Death",
    " of Instant Death",
    " of Misery",
    " of the Whale",
    " of the Lobster",
    " of the Whelk",
    " of the Penguin",
    " of the Puffin",
    " of the Mushroom",
    " of the Toadstool",
/* 90: */
    " of the Little People",
    " of the Puffball",
    " of Spores",
    " of Optimality",
    " of Pareto-Optimality",
    " of Greatest Utility",
    " of Anarcho-Capitalism",
    " of Ancient Evil",
    " of the Revolution",
    " of the People",
/* 100: */
    " of the Elves",
    " of the Dwarves",
    " of the Orcs",
    " of the Humans",
    " of Sludge",
    " of the Naga",
    " of the Trolls",
    " of the Ogres",
    " of Equitable Redistribution",
    " of Wealth",
/* 110: */
    " of Poverty",
    " of Reapportionment",
    " of Fragile Peace",
    " of Reinforcement",
    " of Beauty",
    " of the Slug",
    " of the Snail",
    " of the Gastropod",
    " of Corporal Punishment",
    " of Capital Punishment",
/* 120: */
    " of the Beast",
    " of Light",
    " of Darkness",
    " of Day",
    " of the Day",
    " of Night",
    " of the Night",
    " of Twilight",
    " of the Twilight",
    " of Dawn",
/* 130: */
    " of the Dawn",
    " of the Sun",
    " of the Moon",
    " of Distant Worlds",
    " of the Unseen Realm",
    " of Pandemonium",
    " of the Abyss",
    " of the Nexus",
    " of the Gulag",
    " of the Crusades",
/* 140: */
    " of Proximity",
    " of Wounding",
    " of Peril",
    " of the Eternal Warrior",
    " of the Eternal War",
    " of Evil",
    " of Pounding",
    " of Oozing Pus",
    " of Pestilence",
    " of Plague",
/* 150: */
    " of Negation",
    " of the Saviour",
    " of Infection",
    " of Defence",
    " of Protection",
    " of Defence by Offence",
    " of Expedience",
    " of Reason",
    " of Unreason",
    " of the Heart",
/* 160: */
    " of Offence",
    " of the Leaf",
    " of Leaves",
    " of Winter",
    " of Summer",
    " of Autumn",
    " of Spring",
    " of Midsummer",
    " of Midwinter",
    " of Eternal Night",
/* 170: */
    " of Shrieking Terror",
    " of the Lurker",
    " of the Crawling Thing",
    " of the Thing",
    " \"Thing\"",
    " of the Sea",
    " of the Forest",
    " of the Trees",
    " of Earth",
    " of the World",
/* 180: */
    " of Bread",
    " of Yeast",
    " of the Amoeba",
    " of Deformation",
    " of Guilt",
    " of Innocence",
    " of Ascent",
    " of Descent",
    " of Music",
    " of Brilliance",
/* 190: */
    " of Disgust",
    " of Feasting",
    " of Sunlight",
    " of Starshine",
    " of the Stars",
    " of Dust",
    " of the Clouds",
    " of the Sky",
    " of Ash",
    " of Slime",
/* 200: */
    " of Clarity",
    " of Eternal Vigilance",
    " of Purpose",
    " of the Moth",
    " of the Goat",
    " of Fortitude",
    " of Equivalence",
    " of Balance",
    " of Unbalance",
    " of Harmony",
/* 210: */
    " of Disharmony",
    " of the Inferno",
    " of the Omega Point",
    " of Inflation",
    " of Deflation",
    " of Supply",
    " of Demand",
    " of Gross Domestic Product",
    " of Unjust Enrichment",
    " of Detinue",
/* 220: */
    " of Conversion",
    " of Anton Piller",
    " of Mandamus",
    " of Frustration",
    " of Breach",
    " of Fundamental Breach",
    " of Termination",
    " of Extermination",
    " of Satisfaction",
    " of Res Nullius",
/* 230: */
    " of Fee Simple",
    " of Terra Nullius",
    " of Context",
    " of Prescription",
    " of Freehold",
    " of Tortfeasance",
    " of Omission",
    " of Negligence",
    " of Pains",
    " of Attainder",
/* 240: */
    " of Action",
    " of Inaction",
    " of Truncation",
    " of Defenestration",
    " of Desertification",
    " of the Wilderness",
    " of Psychosis",
    " of Neurosis",
    " of Fixation",
    " of the Open Hand",
/* 250: */
    " of the Tooth",
    " of Honesty",
    " of Dishonesty",
    " of Divine Compulsion",
    " of the Invisible Hand",
    " of Freedom",
    " of Liberty",
    " of Servitude",
    " of Domination",
    " of Tension",
/* 260: */
    " of Monotheism",
    " of Atheism",
    " of Agnosticism",
    " of Existentialism",
    " of the Good",
    " of Relativism",
    " of Absolutism",
    " of Absolution",
    " of Abstinence",
    " of Abomination",
/* 270: */
    " of Mutilation",
    " of Stasis",
    " of Wonder",
    " of Dullness",
    " of Dim Light",
    " of the Shining Light",
    " of Immorality",
    " of Amorality",
    " of Precise Incision",
    " of Orthodoxy",
/* 280: */
    " of Faith",
    " of Untruth",
    " of the Augurer",
    " of the Water Diviner",
    " of the Soothsayer",
    " of Punishment",
    " of Amelioration",
    " of Sulphur",
    " of the Egg",
    " of the Globe",
/* 290: */
    " of the Candle",
    " of the Candelabrum",
    " of the Vampires",
    " of the Orcs",
    " of the Halflings",
    " of World's End",
    " of Blue Skies",
    " of Red Skies",
    " of Orange Skies",
    " of Purple Skies",
/* 300: */
    " of Articulation",
    " of the Mind",
    " of the Spider",
    " of the Lamprey",
    " of the Beginning",
    " of the End",
    " of Severance",
    " of Sequestration",
    " of Mourning",
    " of Death's Door",
/* 310: */
    " of the Key",
    " of Earthquakes",
    " of Failure",
    " of Success",
    " of Intimidation",
    " of the Mosquito",
    " of the Gnat",
    " of the Blowfly",
    " of the Turtle",
    " of the Tortoise",
/* 320: */
    " of the Pit",
    " of the Grave",
    " of Submission",
    " of Dominance",
    " of the Messenger",
    " of Crystal",
    " of Gravity",
    " of Levity",
    " of the Slorg",
    " of Surprise",
/* 330: */
    " of the Maze",
    " of the Labyrinth",
    " of Divine Intervention",
    " of Rotation",
    " of the Spinneret",
    " of the Scorpion",
    " of Demonkind",
    " of the Genius",
    " of Bloodstone",
    " of Grontol",
/* 340: */
    " \"Grim Tooth\"",
    " \"Widowmaker\"",
    " \"Widowermaker\"",
    " \"Lifebane\"",
    " \"Conservator\"",
    " \"Banisher\"",
    " \"Tormentor\"",
    " \"Secret Weapon\"",
    " \"String\"",
    " \"Stringbean\"",
/* 350: */
    " \"Blob\"",
    " \"Globulus\"",
    " \"Hulk\"",
    " \"Raisin\"",
    " \"Starlight\"",
    " \"Giant's Toothpick\"",
    " \"Pendulum\"",
    " \"Backscratcher\"",
    " \"Brush\"",
    " \"Murmur\"",
/* 360: */
    " \"Sarcophage\"",
    " \"Concordance\"",
    " \"Dragon's Tongue\"",
    " \"Arbiter\"",
    " \"Gram\"",
    " \"Grom\"",
    " \"Grim\"",
    " \"Grum\"",
    " \"Rummage\"",
    " \"Omelette\"",
/* 370: */
    " \"Egg\"",
    " \"Aubergine\"",
    " \"Z\"",
    " \"X\"",
    " \"Q\"",
    " \"Ox\"",
    " \"Death Rattle\"",
    " \"Tattletale\"",
    " \"Fish\"",
    " \"Bung\"",
/* 380: */
    " \"Arcanum\"",
    " \"Mud Pie of Death\"",
    " \"Transmigrator\"",
    " \"Ultimatum\"",
    " \"Earthworm\"",
    " \"Worm\"",
    " \"Worm's Wrath\"",
    " \"Xom's Favour\"",
    " \"Bingo\"",
    " \"Leviticus\"",
/* 390: */
    " of Joyful Slaughter"

    // Lemuel's new names
    " \"Eviscerator\"",
    " \"Undertaker\"",
    " \"Embalmer\"",
    " of Sudden Death",
    " of Slow Death",
    " of Certain Death",
    " of Uncertain Death",
    " \"Trog's Wrath\"",
    " \"Ogre's Foe\"",
    " \"Dragon's Doom\"",
    " \"Hellblazer\"",
    " \"Hell-Harrower\"",
    " of Hacking and Slashing",
    " of Anger",
    " of Fury",
    " of Righteous Fury",
    " of the Warrior",
    " of the Doomed Warrior",
    " of the Warrior-Mage",

    // from the crawl.akrasiac.org patch
    " of the Alphagorgon",
    " \"Cookie Cutter\"",

    " of the Nine Deaths",
    " of Megalomania",
    " of Egomania",
    " of Pyrrhic Victory",
    " of Irrepressible Laughter",
    " of Impeachment",
};

static const char *rand_armour_names[] = {
/* 0: */
    " of Shielding",
    " of Grace",
    " of Impermeability",
    " of the Onion",
    " of Life",
    " of Defence",
    " of Nonsense",
    " of Eternal Vigilance",
    " of Fun",
    " of Joy",
    " of Death's Door",
    " of the Gate",
    " of Watchfulness",
    " of Integrity",
    " of Bodily Harmony",
    " of Harmony",
    " of the Untouchables",
    " of Grot",
    " of Grottiness",
    " of Filth",
    " of Wonder",
    " of Wondrous Power",
    " of Power",
    " of Vlad",
    " of the Eternal Fruit",
    " of Invincibility",
    " of Hide-and-Seek",
    " of the Mouse",
    " of the Saviour",
    " of Plasticity",
    " of Baldness",
    " of Terror",
    " of the Arcane",
    " of Resist Death",
    " of Anaesthesia",
    " of the Guardian",
    " of Inviolability",
    " of the Tortoise",
    " of the Turtle",
    " of the Armadillo",
    " of the Echidna",
    " of the Armoured One",
    " of Weirdness",
    " of Pathos",
    " of Serendipity",
    " of Loss",
    " of Hedging",
    " of Indemnity",
    " of Limitation",
    " of Exclusion",
    " of Repulsion",
    " of Untold Secrets",
    " of the Earth",
    " of the Turtledove",
    " of Limited Liability",
    " of Responsibility",
    " of Hadjma",
    " of Glory",
    " of Preservation",
    " of Conservation",
    " of Protective Custody",
    " of the Clam",
    " of the Barnacle",
    " of the Lobster",
    " of Hairiness",
    " of Supple Strength",
    " of Space",
    " of the Vacuum",
    " of Compression",
    " of Decompression",
    " of the Loofah",

    // Lemuel's new names
    " of the Sun",
    " of the Moon",
    " of the Stars",
    " of the Planets",
    " of the Pleiades",
    " of the Morning Star",
    " of the Evening Star",
    " of the Seven Stars",
    " of the Seventy-Seven Stars",
    " of the Seven Hundred Seventy-Seven Stars",
    " of Departure",
    " of Arrival",
    " of Change",
    " of Stasis",
    " of Doubt",
    " of Uncertainty",
    " of the Elephant",
    " of the Donkey",
    " of the Zebra",
    " of the Hippo",
    " of the Giraffe",
    " of the Monkey",
    " of the Monkey's Uncle",
    " of Shadows and Fog",
    " of Love and Death",
    " of Crimes and Misdemeanours",
    " of Courage",
    " of Cowardice",
    " of Valour",
    " of the Extremes",
    " of the Middle",
    " of the Median",
    " of Optimism",
    " of Pessimism",
    " of the Man with No Name",
    " of Crawling",
    " of Zot",
    " of the Orb",
    " of the Orb Guardian",
    " of the Dragon",
    " of the Komodo Dragon",
    " of the Swamp",
    " of the Islands",
    " of the Lair",
    " of the Beasts",
    " of the Vault",
    " of the Hive",
    " of the Beekeeper",
    " of the Garden",
    " of the Gardener",
    " of the Assistant Gardener",
    " of War",
    " of Peace",
    " of Xom",
    " of Xom's Laughter",
    " of Xom's Questionable Humour",
    " of Zin's Displeasure",
    " of the Gods",
    " of Atheism",
    " of Agnosticism",
    " of Heresy",
    " of the Nightingale",
    " of the Meadowlark",
    " of Analysis",
    " of the Ego",
    " of the Id",
    " of the Empty Set",
    " with No Name",
    " with an Unpronounceable Name",
    " with a Strange-Sounding Name",
    " of Trumpets",
    " of the Kettle-Drum",
    " of the Accordion",
    " of the Hangman",
    " of the Hanged Man",
    " of Insatiable Hunger",
    " of the Devil",
    " of Demons",
    " of Lesser Demons",
    " of Greater Demons",
    " of the Iron Devil",
    " of Terror",
    " of Annoyance",
    " of Minor Irritation",
    " of Boredom",
    " of Ennui",
    " of the Springtime",
    " of Midsummer",
    " of the Harvest",
    " of First Snow",
    " of Boiling Hail",
    " of Perpetual Drought",
    " of the Poles",
    " of the Tropics",
    " of the Equator",
    " of the Flat Earth",
    " of Premature Burial",
    " of False Imprisonment",
    " of Moderation",
    " of Extremism",
    " of Fun and Games",
    " of the Great Game",
    " of the Game of Life",
    " of the King",
    " of the Queen",
    " of Royalty",
    " of Nobility",
    " of the Republic",
    " of the Empire",
    " of Commoners",
    " of Peasants",
    " of Townfolk",
    " of the City",
    " of the Country",
    " of the Suburbs",
    " \"Hero's Friend\"",
    " of Calculation",
    " of Instinct",
    " of Intuition",
    " of Magic",
    " of Sorcery",
    " of Hedge Wizardry",
    " of Doom",
    " of Pride",
    " of Honour",
    " of Dishonour",
    " of Shame",
    " of Embarrassment",
    " of Abstract Expressionism",
    " of Free Expression",
    " of the Guilds",
    " of the Guild-Master",
    " of the Apprentice",
    " of the Blacksmith",
    " of the Carpenter",
    " of the Wheelwright",
    " of the Cooper",
    " of the Fisher",
    " of the Hunter",
    " of the Ditch-Digger",
    " of Patience",
    " of Impatience",
    " of Anxiety",
    " of Urgency",
    " \"Forget-Me-Not\"",
    " of Lilacs",
    " of Daffodils",
    " of the Rose",

    // from the crawl.akrasiac.org patch.
    " of the Hot Ocelot",
    " of Eight Boll Weevils",

    " of Internal Strife",
    " of Paranoia",
    " of Claustrophobia",
    " of Agoraphobia",
    " of Dyspraxia",
    " of Mental Paralysis"
};

static const char *randart_weapon_appearance[] = {
    "brightly glowing ", "runed ", "smoking ", "bloodstained ", "twisted ",
    "shimmering ", "warped ", "crystal ", "jewelled ", "transparent ",
    "encrusted ", "pitted ", "slimy ", "polished ", "fine ", "crude ",
    "ancient ", "ichor-stained ", "faintly glowing ", "steaming ", "shiny "
};

static const char *randart_armour_appearance[] = {
    "brightly glowing ", "runed ", "smoking ", "bloodstained ", "twisted ",
    "shimmering ", "warped ", "heavily runed ", "jewelled ",
    "transparent ", "encrusted ", "pitted ", "slimy ", "polished ", "fine ",
    "crude ", "ancient ", "ichor-stained ", "faintly glowing ",
    "steaming ", "shiny ", "distressingly furry "
};

static const char *randart_jewellery_appearance[] = {
    "brightly glowing", "runed", "smoking", "ruby", "twisted",
    "shimmering", "warped", "crystal", "diamond", "transparent",
    "encrusted", "pitted", "slimy", "polished", "fine", "crude",
    "ancient", "emerald", "faintly glowing", "steaming", "shiny",
    "scintillating", "sparkling", "flickering", "glittering"
};

// Remember: disallow unrandart creation in abyss/pan

/*
   The following unrandart bits were taken from $pellbinder's mon-util code
   (see mon-util.h & mon-util.cc) and modified (LRH). They're in randart.cc and
   not randart.h because they're only used in this code module.
*/

struct unrandart_entry
{
    const char *name;        // true name of unrandart (max 31 chars)
    const char *unid_name;   // un-id'd name of unrandart (max 31 chars)

    object_class_type ura_cl;        // class of ura
    int ura_ty;        // type of ura
    int ura_pl;        // plus of ura
    int ura_pl2;       // plus2 of ura
    int ura_col;       // colour of ura
    short prpty[RA_PROPERTIES];

    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip1;
    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip2;
    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip3;
};

static unrandart_entry unranddata[] = {
#include "unrand.h"
};

static FixedVector < bool, NO_UNRANDARTS > unrandart_exist;

static struct unrandart_entry *seekunrandart( const item_def &item );

void set_unrandart_exist(int whun, bool is_exist)
{
    unrandart_exist[whun] = is_exist;
}

bool does_unrandart_exist(int whun)
{
    return (unrandart_exist[whun]);
}

bool is_artefact( const item_def &item )
{
    return (is_random_artefact(item) || is_fixed_artefact(item));
}

// returns true is item is a pure randart or an unrandart
bool is_random_artefact( const item_def &item )
{
    return (item.flags & ISFLAG_ARTEFACT_MASK);
}

// returns true if item in an unrandart
bool is_unrandom_artefact( const item_def &item )
{
    return (item.flags & ISFLAG_UNRANDART);
}

// returns true if item is one of the origional fixed artefacts
bool is_fixed_artefact( const item_def &item )
{
    if (!is_random_artefact( item )
        && item.base_type == OBJ_WEAPONS 
        && item.special >= SPWPN_SINGING_SWORD)
    {
        return (true);
    }

    return (false);
}

unique_item_status_type get_unique_item_status( int base_type, int art )
{
    // Note: for weapons "art" is in item.special,
    //       for orbs it's the sub_type.
    if (base_type == OBJ_WEAPONS)
    {
        if (art >= SPWPN_SINGING_SWORD && art <= SPWPN_SWORD_OF_ZONGULDROK)
            return (you.unique_items[ art - SPWPN_SINGING_SWORD ]);
        else if (art >= SPWPN_SWORD_OF_POWER && art <= SPWPN_STAFF_OF_WUCAD_MU)
            return (you.unique_items[ art - SPWPN_SWORD_OF_POWER + 24 ]);
    }
    else if (base_type == OBJ_ORBS)
    {
        if (art >= 4 && art <= 19)
            return (you.unique_items[ art + 3 ]);

    }

    return (UNIQ_NOT_EXISTS);
}

void set_unique_item_status( int base_type, int art,
                             unique_item_status_type status )
{
    // Note: for weapons "art" is in item.special,
    //       for orbs it's the sub_type.
    if (base_type == OBJ_WEAPONS)
    {
        if (art >= SPWPN_SINGING_SWORD && art <= SPWPN_SWORD_OF_ZONGULDROK)
            you.unique_items[ art - SPWPN_SINGING_SWORD ] = status;
        else if (art >= SPWPN_SWORD_OF_POWER && art <= SPWPN_STAFF_OF_WUCAD_MU)
            you.unique_items[ art - SPWPN_SWORD_OF_POWER + 24 ] = status;
    }
    else if (base_type == OBJ_ORBS)
    {
        if (art >= 4 && art <= 19)
            you.unique_items[ art + 3 ] = status;

    }
}

static long calc_seed( const item_def &item )
{
    return (item.special & RANDART_SEED_MASK);
}

void randart_wpn_properties( const item_def &item, 
                             randart_properties_t &proprt,
                             randart_known_props_t &known)
{
    ASSERT( is_random_artefact( item ) ); 
    ASSERT( item.props.exists( KNOWN_PROPS_KEY ) );
    const CrawlStoreValue &_val = item.props[KNOWN_PROPS_KEY];
    ASSERT( _val.get_type() == SV_VEC );
    const CrawlVector &known_vec = _val.get_vector();
    ASSERT( known_vec.get_type()     == SV_BOOL );
    ASSERT( known_vec.size()         == RA_PROPERTIES);
    ASSERT( known_vec.get_max_size() == RA_PROPERTIES);

    if ( item_ident( item, ISFLAG_KNOW_PROPERTIES ) )
    {
        for (vec_size i = 0; i < RA_PROPERTIES; i++)
            known[i] = (bool) true;
    }
    else
    {
        for (vec_size i = 0; i < RA_PROPERTIES; i++)
            known[i] = known_vec[i];
    }

    const object_class_type aclass = item.base_type;
    const int atype  = item.sub_type;

    int power_level = 0;

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        for (int i = 0; i < RA_PROPERTIES; i++)
            proprt[i] = unrand->prpty[i];

        return;
    }

    const long seed = calc_seed( item );

    rng_save_excursion exc;
    seed_rng( seed );

    if (aclass == OBJ_ARMOUR)
        power_level = item.plus / 2 + 2;
    else if (aclass == OBJ_JEWELLERY)
        power_level = 1 + random2(3) + random2(2);
    else // OBJ_WEAPON
        power_level = item.plus / 3 + item.plus2 / 3;

    if (power_level < 0)
        power_level = 0;

    proprt.init(0);

    if (aclass == OBJ_WEAPONS)  /* Only weapons get brands, of course */
    {
        proprt[RAP_BRAND] = SPWPN_FLAMING + random2(15);        /* brand */

        if (one_chance_in(6))
            proprt[RAP_BRAND] = SPWPN_FLAMING + random2(2);

        if (one_chance_in(6))
            proprt[RAP_BRAND] = SPWPN_ORC_SLAYING + random2(4);

        if (one_chance_in(6))
            proprt[RAP_BRAND] = SPWPN_VORPAL;

        if (proprt[RAP_BRAND] == SPWPN_FLAME
            || proprt[RAP_BRAND] == SPWPN_FROST)
        {
            proprt[RAP_BRAND] = 0;      /* missile wpns */
        }

        if (proprt[RAP_BRAND] == SPWPN_PROTECTION)
            proprt[RAP_BRAND] = 0;      /* no protection */

        if (proprt[RAP_BRAND] == SPWPN_DISRUPTION
            && !(atype == WPN_MACE || atype == WPN_GREAT_MACE
                || atype == WPN_HAMMER))
        {
            proprt[RAP_BRAND] = SPWPN_NORMAL;
        }

        // is this happens, things might get broken -- bwr
        if (proprt[RAP_BRAND] == SPWPN_SPEED && atype == WPN_QUICK_BLADE)
            proprt[RAP_BRAND] = SPWPN_NORMAL;

        if (is_range_weapon(item))
        {
            proprt[RAP_BRAND] = SPWPN_NORMAL;

            if (one_chance_in(3))
            {
                int tmp = random2(20);

                proprt[RAP_BRAND] = (tmp >= 18) ? SPWPN_SPEED :
                                    (tmp >= 14) ? SPWPN_PROTECTION :
                                    (tmp >= 10) ? SPWPN_VENOM                   
                                                : SPWPN_VORPAL + random2(3);
                if (atype == WPN_BLOWGUN
                    && (proprt[RAP_BRAND] == SPWPN_VORPAL
                        || proprt[RAP_BRAND] == SPWPN_VENOM))
                {
                    proprt[RAP_BRAND] = SPWPN_NORMAL;
                }
            }
        }

        if (is_demonic(item))
        {
            switch (random2(9))
            {
            case 0:
                proprt[RAP_BRAND] = SPWPN_DRAINING;
                break;
            case 1:
                proprt[RAP_BRAND] = SPWPN_FLAMING;
                break;
            case 2:
                proprt[RAP_BRAND] = SPWPN_FREEZING;
                break;
            case 3:
                proprt[RAP_BRAND] = SPWPN_ELECTROCUTION;
                break;
            case 4:
                proprt[RAP_BRAND] = SPWPN_VAMPIRICISM;
                break;
            case 5:
                proprt[RAP_BRAND] = SPWPN_PAIN;
                break;
            case 6:
                proprt[RAP_BRAND] = SPWPN_VENOM;
                break;
            default:
                power_level -= 2;
            }
            power_level += 2;
        }
        else if (one_chance_in(3))
            proprt[RAP_BRAND] = SPWPN_NORMAL;
        else
            power_level++;
    }

    if (!one_chance_in(5))
    {
        /* AC mod - not for armours or rings of protection */
        if (one_chance_in(4 + power_level)
            && aclass != OBJ_ARMOUR
            && (aclass != OBJ_JEWELLERY || atype != RING_PROTECTION))
        {
            proprt[RAP_AC] = 1 + random2(3) + random2(3) + random2(3);
            power_level++;
            if (one_chance_in(4))
            {
                proprt[RAP_AC] -= 1 + random2(3) + random2(3) + random2(3);
                power_level--;
            }
        }

        /* ev mod - not for rings of evasion */
        if (one_chance_in(4 + power_level)
            && (aclass != OBJ_JEWELLERY || atype != RING_EVASION))
        {
            proprt[RAP_EVASION] = 1 + random2(3) + random2(3) + random2(3);
            power_level++;
            if (one_chance_in(4))
            {
                proprt[RAP_EVASION] -= 1 + random2(3) + random2(3)
                    + random2(3);
                power_level--;
            }
        }

        /* str mod - not for rings of strength */
        if (one_chance_in(4 + power_level)
            && (aclass != OBJ_JEWELLERY || atype != RING_STRENGTH))
        {
            proprt[RAP_STRENGTH] = 1 + random2(3) + random2(2);
            power_level++;
            if (one_chance_in(4))
            {
                proprt[RAP_STRENGTH] -= 1 + random2(3) + random2(3)
                    + random2(3);
                power_level--;
            }
        }

        /* int mod - not for rings of intelligence */
        if (one_chance_in(4 + power_level)
            && (aclass != OBJ_JEWELLERY || atype != RING_INTELLIGENCE))
        {
            proprt[RAP_INTELLIGENCE] = 1 + random2(3) + random2(2);
            power_level++;
            if (one_chance_in(4))
            {
                proprt[RAP_INTELLIGENCE] -= 1 + random2(3) + random2(3)
                    + random2(3);
                power_level--;
            }
        }

        /* dex mod - not for rings of dexterity */
        if (one_chance_in(4 + power_level)
            && (aclass != OBJ_JEWELLERY || atype != RING_DEXTERITY))
        {
            proprt[RAP_DEXTERITY] = 1 + random2(3) + random2(2);
            power_level++;
            if (one_chance_in(4))
            {
                proprt[RAP_DEXTERITY] -= 1 + random2(3) + random2(3)
                    + random2(3);
                power_level--;
            }
        }
    }

    if (random2(15) >= power_level && aclass != OBJ_WEAPONS &&
        (aclass != OBJ_JEWELLERY || atype != RING_SLAYING))
    {
        /* Weapons and rings of slaying can't get these */
        if (one_chance_in(4 + power_level))  /* to-hit */
        {
            proprt[RAP_ACCURACY] = 1 + random2(3) + random2(2);
            power_level++;
            if (one_chance_in(4))
            {
                proprt[RAP_ACCURACY] -= 1 + random2(3) + random2(3) +
                    random2(3);
                power_level--;
            }
        }

        if (one_chance_in(4 + power_level))  /* to-dam */
        {
            proprt[RAP_DAMAGE] = 1 + random2(3) + random2(2);
            power_level++;
            if (one_chance_in(4))
            {
                proprt[RAP_DAMAGE] -= 1 + random2(3) + random2(3) + random2(3);
                power_level--;
            }
        }
    }

    bool done_powers = (random2(12 < power_level));

    /* res_fire */
    if (!done_powers
        && one_chance_in(4 + power_level)
        && (aclass != OBJ_JEWELLERY
            || (atype != RING_PROTECTION_FROM_FIRE
                && atype != RING_FIRE
                && atype != RING_ICE))
        && (aclass != OBJ_ARMOUR
            || (atype != ARM_DRAGON_ARMOUR
                && atype != ARM_ICE_DRAGON_ARMOUR
                && atype != ARM_GOLD_DRAGON_ARMOUR)))
    {
        proprt[RAP_FIRE] = 1;
        if (one_chance_in(5))
            proprt[RAP_FIRE]++;
        power_level++;
    }

    /* res_cold */
    if (!done_powers
        && one_chance_in(4 + power_level)
        && (aclass != OBJ_JEWELLERY
            || (atype != RING_PROTECTION_FROM_COLD
                && atype != RING_FIRE
                && atype != RING_ICE))
        && (aclass != OBJ_ARMOUR
            || (atype != ARM_DRAGON_ARMOUR
                && atype != ARM_ICE_DRAGON_ARMOUR
                && atype != ARM_GOLD_DRAGON_ARMOUR)))
    {
        proprt[RAP_COLD] = 1;
        if (one_chance_in(5))
            proprt[RAP_COLD]++;
        power_level++;
    }

    if (random2(12) < power_level || power_level > 7)
        done_powers = true;

    /* res_elec */
    if (!done_powers
        && one_chance_in(4 + power_level)
        && (aclass != OBJ_ARMOUR || atype != ARM_STORM_DRAGON_ARMOUR))
    {
        proprt[RAP_ELECTRICITY] = 1;
        power_level++;
    }

    /* res_poison */
    if (!done_powers
        && one_chance_in(5 + power_level)
        && (aclass != OBJ_JEWELLERY || atype != RING_POISON_RESISTANCE)
        && (aclass != OBJ_ARMOUR
            || atype != ARM_GOLD_DRAGON_ARMOUR
            || atype != ARM_SWAMP_DRAGON_ARMOUR))
    {
        proprt[RAP_POISON] = 1;
        power_level++;
    }

    /* prot_life - no necromantic brands on weapons allowed */
    if (!done_powers
        && one_chance_in(4 + power_level)
        && (aclass != OBJ_JEWELLERY || atype != RING_TELEPORTATION)
        && proprt[RAP_BRAND] != SPWPN_DRAINING
        && proprt[RAP_BRAND] != SPWPN_VAMPIRICISM
        && proprt[RAP_BRAND] != SPWPN_PAIN)
    {
        proprt[RAP_NEGATIVE_ENERGY] = 1;
        power_level++;
    }

    /* res magic */
    if (!done_powers
        && one_chance_in(4 + power_level)
        && (aclass != OBJ_JEWELLERY || atype != RING_PROTECTION_FROM_MAGIC))
    {
        proprt[RAP_MAGIC] = 35 + random2(65);
        power_level++;
    }

    /* see_invis */
    if (!done_powers
        && one_chance_in(4 + power_level)
        && (aclass != OBJ_JEWELLERY || atype != RING_INVISIBILITY))
    {
        proprt[RAP_EYESIGHT] = 1;
        power_level++;
    }

    if (random2(12) < power_level || power_level > 10)
        done_powers = true;

    /* turn invis */
    if (!done_powers
        && one_chance_in(10)
        && (aclass != OBJ_JEWELLERY || atype != RING_INVISIBILITY))
    {
        proprt[RAP_INVISIBLE] = 1;
        power_level++;
    }

    /* levitate */
    if (!done_powers
        && one_chance_in(10)
        && (aclass != OBJ_JEWELLERY || atype != RING_LEVITATION))
    {
        proprt[RAP_LEVITATE] = 1;
        power_level++;
    }

    if (!done_powers && one_chance_in(10))       /* blink */
    {
        proprt[RAP_BLINK] = 1;
        power_level++;
    }

    /* teleport */
    if (!done_powers
        && one_chance_in(10)
        && (aclass != OBJ_JEWELLERY || atype != RING_TELEPORTATION))
    {
        proprt[RAP_CAN_TELEPORT] = 1;
        power_level++;
    }

    /* go berserk */
    if (!done_powers
        && one_chance_in(10)
        && (aclass != OBJ_JEWELLERY || atype != AMU_RAGE))
    {
        proprt[RAP_BERSERK] = 1;
        power_level++;
    }

    if (!done_powers && one_chance_in(10))       /* sense surr */
    {
        proprt[RAP_MAPPING] = 1;
        power_level++;
    }

    /* Armours get less powers, and are also less likely to be
       cursed that wpns */
    if (aclass == OBJ_ARMOUR)
        power_level -= 4;

    if (power_level >= 2 && random2(17) < power_level)
    {
        switch (random2(9))
        {
        case 0:                     /* makes noise */
            if (aclass != OBJ_WEAPONS)
                break;
            proprt[RAP_NOISES] = 1 + random2(4);
            break;
        case 1:                     /* no magic */
            proprt[RAP_PREVENT_SPELLCASTING] = 1;
            break;
        case 2:                     /* random teleport */
            if (aclass != OBJ_WEAPONS)
                break;
            proprt[RAP_CAUSE_TELEPORTATION] = 5 + random2(15);
            break;
        case 3:   /* no teleport - doesn't affect some instantaneous
                   * teleports */
            if (aclass == OBJ_JEWELLERY && atype == RING_TELEPORTATION)
                break;              /* already is a ring of tport */
            if (aclass == OBJ_JEWELLERY && atype == RING_TELEPORT_CONTROL)
                break;              /* already is a ring of tport ctrl */
            proprt[RAP_BLINK] = 0;
            proprt[RAP_CAN_TELEPORT] = 0;
            proprt[RAP_PREVENT_TELEPORTATION] = 1;
            break;
        case 4:                     /* berserk on attack */
            if (aclass != OBJ_WEAPONS)
                break;
            proprt[RAP_ANGRY] = 1 + random2(8);
            break;
        case 5:                     /* susceptible to fire */
            if (aclass == OBJ_JEWELLERY
                && (atype == RING_PROTECTION_FROM_FIRE || atype == RING_FIRE
                    || atype == RING_ICE))
                break;              /* already does this or something */
            if (aclass == OBJ_ARMOUR
                && (atype == ARM_DRAGON_ARMOUR || atype == ARM_ICE_DRAGON_ARMOUR
                    || atype == ARM_GOLD_DRAGON_ARMOUR))
                break;
            proprt[RAP_FIRE] = -1;
            break;
        case 6:                     /* susceptible to cold */
            if (aclass == OBJ_JEWELLERY
                && (atype == RING_PROTECTION_FROM_COLD || atype == RING_FIRE
                    || atype == RING_ICE))
                break;              /* already does this or something */
            if (aclass == OBJ_ARMOUR
                && (atype == ARM_DRAGON_ARMOUR || atype == ARM_ICE_DRAGON_ARMOUR
                    || atype == ARM_GOLD_DRAGON_ARMOUR))
                break;
            proprt[RAP_COLD] = -1;
            break;
        case 7:                     /* speed metabolism */
            if (aclass == OBJ_JEWELLERY && atype == RING_HUNGER)
                break;              /* already is a ring of hunger */
            if (aclass == OBJ_JEWELLERY && atype == RING_SUSTENANCE)
                break;              /* already is a ring of sustenance */
            proprt[RAP_METABOLISM] = 1 + random2(3);
            break;
        case 8:
            /* emits mutagenic radiation - increases
               magic_contamination.  property is chance (1 in ...) of
               increasing magic_contamination */
            proprt[RAP_MUTAGENIC] = 2 + random2(4);
            break;
        }
    }

    if (one_chance_in(10) 
        && (aclass != OBJ_ARMOUR 
            || atype != ARM_CLOAK 
            || get_equip_race(item) != ISFLAG_ELVEN)
        && (aclass != OBJ_ARMOUR 
            || atype != ARM_BOOTS 
            || get_equip_race(item) != ISFLAG_ELVEN)
        && get_armour_ego_type( item ) != SPARM_STEALTH)
    {
        power_level++;
        proprt[RAP_STEALTH] = 10 + random2(70);

        if (one_chance_in(4))
        {
            proprt[RAP_STEALTH] = -proprt[RAP_STEALTH] - random2(20);    
            power_level--;
        }
    }

    if ((power_level < 2 && one_chance_in(5)) || one_chance_in(30))
        proprt[RAP_CURSED] = 1;
}

void randart_wpn_properties( const item_def &item, 
                             randart_properties_t &proprt )
{
    randart_known_props_t known;

    randart_wpn_properties(item, proprt, known);
}

int randart_wpn_property( const item_def &item, int prop, bool &_known )
{
    randart_properties_t  proprt;
    randart_known_props_t known;

    randart_wpn_properties( item, proprt, known );

    _known = known[prop];

    return ( proprt[prop] );
}

int randart_wpn_property( const item_def &item, int prop )
{
    bool known;

    return randart_wpn_property( item, prop, known );
}

void randart_wpn_learn_prop( item_def &item, int prop )
{
    ASSERT( is_random_artefact( item ) ); 
    ASSERT( item.props.exists( KNOWN_PROPS_KEY ) );
    CrawlStoreValue &_val = item.props[KNOWN_PROPS_KEY];
    ASSERT( _val.get_type() == SV_VEC );
    CrawlVector &known_vec = _val.get_vector();
    ASSERT( known_vec.get_type()     == SV_BOOL );
    ASSERT( known_vec.size()         == RA_PROPERTIES);
    ASSERT( known_vec.get_max_size() == RA_PROPERTIES);

    if ( item_ident( item, ISFLAG_KNOW_PROPERTIES ) )
        return;
    else
        known_vec[prop] = (bool) true;
}

bool randart_wpn_known_prop( const item_def &item, int prop )
{
    bool known;
    randart_wpn_property( item, prop, known );
    return known;
}

std::string randart_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_WEAPONS );

    if (is_unrandom_artefact( item ))
    {
        const struct unrandart_entry *unrand = seekunrandart( item );
        return (item_type_known(item) ? unrand->name : unrand->unid_name);
    }

    const long seed = calc_seed( item );

    rng_save_excursion rng_state;
    seed_rng( seed );
    
    std::string result;
    
    if (!item_type_known(item))
    {
        result += RANDOM_ELEMENT(randart_weapon_appearance);
        result += item_base_name(item);
        return result;
    }

    if (coinflip())
    {
        result += item_base_name(item);
        result += RANDOM_ELEMENT(rand_wpn_names);
    }
    else
    {
        const std::string st_p = make_name(random_int(), false);
        result += item_base_name(item);

        if (one_chance_in(3))
        {
            result += " of ";
            result += st_p;
        }
        else
        {
            result += " \"";
            result += st_p;
            result += "\"";
        }
    }

    return result;
}

std::string randart_armour_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_ARMOUR );

    if (is_unrandom_artefact( item ))
    {
        const struct unrandart_entry *unrand = seekunrandart( item );
        return (item_type_known(item) ? unrand->name : unrand->unid_name);
    }

    const long seed = calc_seed( item );

    rng_save_excursion exc;
    seed_rng( seed );

    std::string result;

    if (!item_type_known(item))
    {
        result += RANDOM_ELEMENT(randart_armour_appearance);
        result += item_base_name(item);
        return result;
    }
    
    if (coinflip())
    {
        result += item_base_name(item);
        result += RANDOM_ELEMENT(rand_armour_names);
    }
    else
    {
        const std::string st_p = make_name(random_int(), false);
        result += item_base_name(item);
        if (one_chance_in(3))
        {
            result += " of ";
            result += st_p;
        }
        else
        {
            result += " \"";
            result += st_p;
            result += "\"";
        }
    }

    return result;
}

std::string randart_jewellery_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_JEWELLERY );


    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        return (item_type_known(item) ? unrand->name
                                      : unrand->unid_name);
    }

    const long seed = calc_seed( item );

    rng_save_excursion exc;
    seed_rng( seed );

    std::string result;

    if (!item_type_known(item))
    {
        result += RANDOM_ELEMENT(randart_jewellery_appearance);
        result += " ";
        result += (jewellery_is_amulet(item) ? "amulet" : "ring");

        return result;
    }

    if (one_chance_in(5))
    {
        result += (jewellery_is_amulet(item) ? "amulet" : "ring");
        result += RANDOM_ELEMENT(rand_armour_names);
    }
    else
    {
        const std::string st_p = make_name(random_int(), false);
        result += (jewellery_is_amulet(item) ? "amulet" : "ring");

        if (one_chance_in(3))
        {
            result += " of ";
            result += st_p;
        }
        else
        {
            result += " \"";
            result += st_p;
            result += "\"";
        }
    }

    return result;
}

static struct unrandart_entry *seekunrandart( const item_def &item )
{
    int x = 0;

    while (x < NO_UNRANDARTS)
    {
        if (unranddata[x].ura_cl == item.base_type 
            && unranddata[x].ura_ty == item.sub_type
            && unranddata[x].ura_pl == item.plus 
            && unranddata[x].ura_pl2 == item.plus2
            && unranddata[x].ura_col == item.colour)
        {
            return (&unranddata[x]);
        }

        x++;
    }

    return (&unranddata[0]);  // Dummy object
}                               // end seekunrandart()

int find_unrandart_index(int item_number)
{
    int x;

    for(x=0; x < NO_UNRANDARTS; x++)
    {
        if (unranddata[x].ura_cl == mitm[item_number].base_type
            && unranddata[x].ura_ty == mitm[item_number].sub_type
            && unranddata[x].ura_pl == mitm[item_number].plus
            && unranddata[x].ura_pl2 == mitm[item_number].plus2)
        {
            return (x);
        }
    }

    return (-1);
}

int find_okay_unrandart(unsigned char aclass, unsigned char atype)
{
    int x, count;
    int ret = -1;

    for (x = 0, count = 0; x < NO_UNRANDARTS; x++)
    {
        if (unranddata[x].ura_cl == aclass 
            && !does_unrandart_exist(x)
            && (atype == OBJ_RANDOM || unranddata[x].ura_ty == atype))
        {
            count++;

            if (one_chance_in(count))
                ret = x;
        }
    }

    return (ret);
}                               // end find_okay_unrandart()

// which == 0 (default) gives random fixed artefact.
// Returns true if successful.
bool make_item_fixed_artefact( item_def &item, bool in_abyss, int which )
{
    bool  force = true;  // we force any one asked for specifically

    if (!which)
    {
        // using old behaviour... try only once. -- bwr
        force = false;  

        which = SPWPN_SINGING_SWORD + random2(12);
        if (which >= SPWPN_SWORD_OF_CEREBOV)
            which += 3; // skip over Cerebov's, Dispater's, and Asmodeus' weapons
    }

    int status = get_unique_item_status( OBJ_WEAPONS, which );

    if ((status == UNIQ_EXISTS 
            || (in_abyss && status == UNIQ_NOT_EXISTS)
            || (!in_abyss && status == UNIQ_LOST_IN_ABYSS))
        && !force)
    {
        return (false);
    }

    switch (which)
    {
    case SPWPN_SINGING_SWORD:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_LONG_SWORD;
        item.plus  = 7;
        item.plus2 = 6;
        break;

    case SPWPN_WRATH_OF_TROG:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_BATTLEAXE;
        item.plus  = 3;
        item.plus2 = 11;
        break;

    case SPWPN_SCYTHE_OF_CURSES:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_SCYTHE;
        item.plus  = 13;
        item.plus2 = 13;
        break;

    case SPWPN_MACE_OF_VARIABILITY:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_MACE;
        item.plus  = random2(16) - 4;
        item.plus2 = random2(16) - 4;
        break;

    case SPWPN_GLAIVE_OF_PRUNE:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GLAIVE;
        item.plus  = 0;
        item.plus2 = 12;
        break;

    case SPWPN_SCEPTRE_OF_TORMENT:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_MACE;
        item.plus  = 7;
        item.plus2 = 6;
        break;

    case SPWPN_SWORD_OF_ZONGULDROK:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_LONG_SWORD;
        item.plus  = 9;
        item.plus2 = 9;
        break;

    case SPWPN_SWORD_OF_POWER:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GREAT_SWORD;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_KNIFE_OF_ACCURACY:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_DAGGER;
        item.plus  = 27;
        item.plus2 = -1;
        break;

    case SPWPN_STAFF_OF_OLGREB:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_VAMPIRES_TOOTH:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_DAGGER;
        item.plus  = 3;
        item.plus2 = 4;
        break;

    case SPWPN_STAFF_OF_WUCAD_MU:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_SWORD_OF_CEREBOV:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GREAT_SWORD;
        item.plus  = 6;
        item.plus2 = 6;
        item.colour = YELLOW;
        do_curse_item( item );
        break;

    case SPWPN_STAFF_OF_DISPATER:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 4;
        item.plus2 = 4;
        item.colour = YELLOW;
        break;

    case SPWPN_SCEPTRE_OF_ASMODEUS:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 7;
        item.plus2 = 7;
        item.colour = RED;
        break;

    default:
        DEBUGSTR( "Trying to create illegal fixed artefact!" );
        return (false);
    }

    // If we get here, we've made the artefact
    item.special = which;
    item.quantity = 1;

    // Items originally generated in the abyss and not found will be 
    // shifted to "lost in abyss", and will only be found there. -- bwr
    set_unique_item_status( OBJ_WEAPONS, which, UNIQ_EXISTS );

    return (true);
}

bool make_item_randart( item_def &item )
{
    if (item.base_type != OBJ_WEAPONS 
        && item.base_type != OBJ_ARMOUR
        && item.base_type != OBJ_JEWELLERY)
    {
        return (false);
    }

    if (item.flags & ISFLAG_RANDART)
    {
        return (true);
    }

    if (item.flags & ISFLAG_UNRANDART)
    {
#if DEBUG
        mprf(MSGCH_DIAGNOSTICS, "Trying to turn '%s' from an unrandart to"
             "a randart.", item.name(DESC_PLAIN, false, true).c_str());
#endif
        return (false);
    }

    item.flags  |= ISFLAG_RANDART;
    item.special = (random_int() & RANDART_SEED_MASK);

    ASSERT(!item.props.exists( KNOWN_PROPS_KEY ));

    item.props[KNOWN_PROPS_KEY].new_vector(SV_BOOL).resize(RA_PROPERTIES);
    CrawlVector &known = item.props[KNOWN_PROPS_KEY];
    known.set_max_size(RA_PROPERTIES);
    for (vec_size i = 0; i < RA_PROPERTIES; i++)
        known[i] = (bool) false;

    return (true);
}

// void make_item_unrandart( int x, int ura_item )
bool make_item_unrandart( item_def &item, int unrand_index )
{
    item.base_type = unranddata[unrand_index].ura_cl;
    item.sub_type  = unranddata[unrand_index].ura_ty;
    item.plus      = unranddata[unrand_index].ura_pl;
    item.plus2     = unranddata[unrand_index].ura_pl2;
    item.colour    = unranddata[unrand_index].ura_col;

    item.flags |= ISFLAG_UNRANDART; 
    item.special = unranddata[ unrand_index ].prpty[ RAP_BRAND ];

    if (unranddata[ unrand_index ].prpty[ RAP_CURSED ])
        do_curse_item( item );

    set_unrandart_exist( unrand_index, 1 );

    return (true);
}                               // end make_item_unrandart()

const char *unrandart_descrip( char which_descrip, const item_def &item )
{
/* Eventually it would be great to have randomly generated descriptions for
   randarts. */
    struct unrandart_entry *unrand = seekunrandart( item );

    return ((which_descrip == 0) ? unrand->spec_descrip1 :
            (which_descrip == 1) ? unrand->spec_descrip2 :
            (which_descrip == 2) ? unrand->spec_descrip3 : "Unknown.");

}                               // end unrandart_descrip()
