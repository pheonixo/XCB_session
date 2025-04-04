
struct _xkeynaming {
  char *name;
  unsigned short keyval;
};

static struct _xkeynaming knames[] = {
/*
 * Latin 1
 * (ISO/IEC 8859-1 = Unicode U+0020..U+00FF)
 * Byte 3 = 0
 */

 { "XK_space",                       0x0020 },  /* U+0020 SPACE */
 { "XK_exclam",                      0x0021 },  /* U+0021 EXCLAMATION MARK */
 { "XK_quotedbl",                    0x0022 },  /* U+0022 QUOTATION MARK */
 { "XK_numbersign",                  0x0023 },  /* U+0023 NUMBER SIGN */
 { "XK_dollar",                      0x0024 },  /* U+0024 DOLLAR SIGN */
 { "XK_percent",                     0x0025 },  /* U+0025 PERCENT SIGN */
 { "XK_ampersand",                   0x0026 },  /* U+0026 AMPERSAND */
 { "XK_apostrophe",                  0x0027 },  /* U+0027 APOSTROPHE */
/* { "XK_quoteright",                  0x0027 },   deprecated */
 { "XK_parenleft",                   0x0028 },  /* U+0028 LEFT PARENTHESIS */
 { "XK_parenright",                  0x0029 },  /* U+0029 RIGHT PARENTHESIS */
 { "XK_asterisk",                    0x002a },  /* U+002A ASTERISK */
 { "XK_plus",                        0x002b },  /* U+002B PLUS SIGN */
 { "XK_comma",                       0x002c },  /* U+002C COMMA */
 { "XK_minus",                       0x002d },  /* U+002D HYPHEN-MINUS */
 { "XK_period",                      0x002e },  /* U+002E FULL STOP */
 { "XK_slash",                       0x002f },  /* U+002F SOLIDUS */
 { "XK_0",                           0x0030 },  /* U+0030 DIGIT ZERO */
 { "XK_1",                           0x0031 },  /* U+0031 DIGIT ONE */
 { "XK_2",                           0x0032 },  /* U+0032 DIGIT TWO */
 { "XK_3",                           0x0033 },  /* U+0033 DIGIT THREE */
 { "XK_4",                           0x0034 },  /* U+0034 DIGIT FOUR */
 { "XK_5",                           0x0035 },  /* U+0035 DIGIT FIVE */
 { "XK_6",                           0x0036 },  /* U+0036 DIGIT SIX */
 { "XK_7",                           0x0037 },  /* U+0037 DIGIT SEVEN */
 { "XK_8",                           0x0038 },  /* U+0038 DIGIT EIGHT */
 { "XK_9",                           0x0039 },  /* U+0039 DIGIT NINE */
 { "XK_colon",                       0x003a },  /* U+003A COLON */
 { "XK_semicolon",                   0x003b },  /* U+003B SEMICOLON */
 { "XK_less",                        0x003c },  /* U+003C LESS-THAN SIGN */
 { "XK_equal",                       0x003d },  /* U+003D EQUALS SIGN */
 { "XK_greater",                     0x003e },  /* U+003E GREATER-THAN SIGN */
 { "XK_question",                    0x003f },  /* U+003F QUESTION MARK */
 { "XK_at",                          0x0040 },  /* U+0040 COMMERCIAL AT */
 { "XK_A",                           0x0041 },  /* U+0041 LATIN CAPITAL LETTER A */
 { "XK_B",                           0x0042 },  /* U+0042 LATIN CAPITAL LETTER B */
 { "XK_C",                           0x0043 },  /* U+0043 LATIN CAPITAL LETTER C */
 { "XK_D",                           0x0044 },  /* U+0044 LATIN CAPITAL LETTER D */
 { "XK_E",                           0x0045 },  /* U+0045 LATIN CAPITAL LETTER E */
 { "XK_F",                           0x0046 },  /* U+0046 LATIN CAPITAL LETTER F */
 { "XK_G",                           0x0047 },  /* U+0047 LATIN CAPITAL LETTER G */
 { "XK_H",                           0x0048 },  /* U+0048 LATIN CAPITAL LETTER H */
 { "XK_I",                           0x0049 },  /* U+0049 LATIN CAPITAL LETTER I */
 { "XK_J",                           0x004a },  /* U+004A LATIN CAPITAL LETTER J */
 { "XK_K",                           0x004b },  /* U+004B LATIN CAPITAL LETTER K */
 { "XK_L",                           0x004c },  /* U+004C LATIN CAPITAL LETTER L */
 { "XK_M",                           0x004d },  /* U+004D LATIN CAPITAL LETTER M */
 { "XK_N",                           0x004e },  /* U+004E LATIN CAPITAL LETTER N */
 { "XK_O",                           0x004f },  /* U+004F LATIN CAPITAL LETTER O */
 { "XK_P",                           0x0050 },  /* U+0050 LATIN CAPITAL LETTER P */
 { "XK_Q",                           0x0051 },  /* U+0051 LATIN CAPITAL LETTER Q */
 { "XK_R",                           0x0052 },  /* U+0052 LATIN CAPITAL LETTER R */
 { "XK_S",                           0x0053 },  /* U+0053 LATIN CAPITAL LETTER S */
 { "XK_T",                           0x0054 },  /* U+0054 LATIN CAPITAL LETTER T */
 { "XK_U",                           0x0055 },  /* U+0055 LATIN CAPITAL LETTER U */
 { "XK_V",                           0x0056 },  /* U+0056 LATIN CAPITAL LETTER V */
 { "XK_W",                           0x0057 },  /* U+0057 LATIN CAPITAL LETTER W */
 { "XK_X",                           0x0058 },  /* U+0058 LATIN CAPITAL LETTER X */
 { "XK_Y",                           0x0059 },  /* U+0059 LATIN CAPITAL LETTER Y */
 { "XK_Z",                           0x005a },  /* U+005A LATIN CAPITAL LETTER Z */
 { "XK_bracketleft",                 0x005b },  /* U+005B LEFT SQUARE BRACKET */
 { "XK_backslash",                   0x005c },  /* U+005C REVERSE SOLIDUS */
 { "XK_bracketright",                0x005d },  /* U+005D RIGHT SQUARE BRACKET */
 { "XK_asciicircum",                 0x005e },  /* U+005E CIRCUMFLEX ACCENT */
 { "XK_underscore",                  0x005f },  /* U+005F LOW LINE */
 { "XK_grave",                       0x0060 },  /* U+0060 GRAVE ACCENT */
/* { "XK_quoteleft",                   0x0060 },   deprecated */
 { "XK_a",                           0x0061 },  /* U+0061 LATIN SMALL LETTER A */
 { "XK_b",                           0x0062 },  /* U+0062 LATIN SMALL LETTER B */
 { "XK_c",                           0x0063 },  /* U+0063 LATIN SMALL LETTER C */
 { "XK_d",                           0x0064 },  /* U+0064 LATIN SMALL LETTER D */
 { "XK_e",                           0x0065 },  /* U+0065 LATIN SMALL LETTER E */
 { "XK_f",                           0x0066 },  /* U+0066 LATIN SMALL LETTER F */
 { "XK_g",                           0x0067 },  /* U+0067 LATIN SMALL LETTER G */
 { "XK_h",                           0x0068 },  /* U+0068 LATIN SMALL LETTER H */
 { "XK_i",                           0x0069 },  /* U+0069 LATIN SMALL LETTER I */
 { "XK_j",                           0x006a },  /* U+006A LATIN SMALL LETTER J */
 { "XK_k",                           0x006b },  /* U+006B LATIN SMALL LETTER K */
 { "XK_l",                           0x006c },  /* U+006C LATIN SMALL LETTER L */
 { "XK_m",                           0x006d },  /* U+006D LATIN SMALL LETTER M */
 { "XK_n",                           0x006e },  /* U+006E LATIN SMALL LETTER N */
 { "XK_o",                           0x006f },  /* U+006F LATIN SMALL LETTER O */
 { "XK_p",                           0x0070 },  /* U+0070 LATIN SMALL LETTER P */
 { "XK_q",                           0x0071 },  /* U+0071 LATIN SMALL LETTER Q */
 { "XK_r",                           0x0072 },  /* U+0072 LATIN SMALL LETTER R */
 { "XK_s",                           0x0073 },  /* U+0073 LATIN SMALL LETTER S */
 { "XK_t",                           0x0074 },  /* U+0074 LATIN SMALL LETTER T */
 { "XK_u",                           0x0075 },  /* U+0075 LATIN SMALL LETTER U */
 { "XK_v",                           0x0076 },  /* U+0076 LATIN SMALL LETTER V */
 { "XK_w",                           0x0077 },  /* U+0077 LATIN SMALL LETTER W */
 { "XK_x",                           0x0078 },  /* U+0078 LATIN SMALL LETTER X */
 { "XK_y",                           0x0079 },  /* U+0079 LATIN SMALL LETTER Y */
 { "XK_z",                           0x007a },  /* U+007A LATIN SMALL LETTER Z */
 { "XK_braceleft",                   0x007b },  /* U+007B LEFT CURLY BRACKET */
 { "XK_bar",                         0x007c },  /* U+007C VERTICAL LINE */
 { "XK_braceright",                  0x007d },  /* U+007D RIGHT CURLY BRACKET */
 { "XK_asciitilde",                  0x007e },  /* U+007E TILDE */

/* { "Unmapped",                       0x007f - 0xfd00 }, */
/*
 * 3270 Terminal Keys
 * Byte 3 = 0xfd
 */

 { "XK_3270_Duplicate",              0xfd01 },
 { "XK_3270_FieldMark",              0xfd02 },
 { "XK_3270_Right2",                 0xfd03 },
 { "XK_3270_Left2",                  0xfd04 },
 { "XK_3270_BackTab",                0xfd05 },
 { "XK_3270_EraseEOF",               0xfd06 },
 { "XK_3270_EraseInput",             0xfd07 },
 { "XK_3270_Reset",                  0xfd08 },
 { "XK_3270_Quit",                   0xfd09 },
 { "XK_3270_PA1",                    0xfd0a },
 { "XK_3270_PA2",                    0xfd0b },
 { "XK_3270_PA3",                    0xfd0c },
 { "XK_3270_Test",                   0xfd0d },
 { "XK_3270_Attn",                   0xfd0e },
 { "XK_3270_CursorBlink",            0xfd0f },
 { "XK_3270_AltCursor",              0xfd10 },
 { "XK_3270_KeyClick",               0xfd11 },
 { "XK_3270_Jump",                   0xfd12 },
 { "XK_3270_Ident",                  0xfd13 },
 { "XK_3270_Rule",                   0xfd14 },
 { "XK_3270_Copy",                   0xfd15 },
 { "XK_3270_Play",                   0xfd16 },
 { "XK_3270_Setup",                  0xfd17 },
 { "XK_3270_Record",                 0xfd18 },
 { "XK_3270_ChangeScreen",           0xfd19 },
 { "XK_3270_DeleteWord",             0xfd1a },
 { "XK_3270_ExSelect",               0xfd1b },
 { "XK_3270_CursorSelect",           0xfd1c },
 { "XK_3270_PrintScreen",            0xfd1d },
 { "XK_3270_Enter",                  0xfd1e },

/*
 * Keyboard (XKB) Extension function and modifier keys
 * (from Appendix C of "The X Keyboard Extension: Protocol Specification")
 * Byte 3 = 0xfe
 */
/* { "Unmapped",                       0xfd1f - 0xfe00 }, */

 { "XK_ISO_Lock",                    0xfe01 },
 { "XK_ISO_Level2_Latch",            0xfe02 },
 { "XK_ISO_Level3_Shift",            0xfe03 },
 { "XK_ISO_Level3_Latch",            0xfe04 },
 { "XK_ISO_Level3_Lock",             0xfe05 },
 { "XK_ISO_Group_Latch",             0xfe06 },
 { "XK_ISO_Group_Lock",              0xfe07 },
 { "XK_ISO_Next_Group",              0xfe08 },
 { "XK_ISO_Next_Group_Lock",         0xfe09 },
 { "XK_ISO_Prev_Group",              0xfe0a },
 { "XK_ISO_Prev_Group_Lock",         0xfe0b },
 { "XK_ISO_First_Group",             0xfe0c },
 { "XK_ISO_First_Group_Lock",        0xfe0d },
 { "XK_ISO_Last_Group",              0xfe0e },
 { "XK_ISO_Last_Group_Lock",         0xfe0f },
 { "Unmapped",                       0xfe10 },
 { "XK_ISO_Level5_Shift",            0xfe11 },
 { "XK_ISO_Level5_Latch",            0xfe12 },
 { "XK_ISO_Level5_Lock",             0xfe13 },

 { "Unmapped",                       0xfe14 },
 { "Unmapped",                       0xfe15 },
 { "Unmapped",                       0xfe16 },
 { "Unmapped",                       0xfe17 },
 { "Unmapped",                       0xfe18 },
 { "Unmapped",                       0xfe19 },
 { "Unmapped",                       0xfe1a },
 { "Unmapped",                       0xfe1b },
 { "Unmapped",                       0xfe1c },
 { "Unmapped",                       0xfe1d },
 { "Unmapped",                       0xfe1e },
 { "Unmapped",                       0xfe1f },

 { "XK_ISO_Left_Tab",                0xfe20 },
 { "XK_ISO_Move_Line_Up",            0xfe21 },
 { "XK_ISO_Move_Line_Down",          0xfe22 },
 { "XK_ISO_Partial_Line_Up",         0xfe23 },
 { "XK_ISO_Partial_Line_Down",       0xfe24 },
 { "XK_ISO_Partial_Space_Left",      0xfe25 },
 { "XK_ISO_Partial_Space_Right",     0xfe26 },
 { "XK_ISO_Set_Margin_Left",         0xfe27 },
 { "XK_ISO_Set_Margin_Right",        0xfe28 },
 { "XK_ISO_Release_Margin_Left",     0xfe29 },
 { "XK_ISO_Release_Margin_Right",    0xfe2a },
 { "XK_ISO_Release_Both_Margins",    0xfe2b },
 { "XK_ISO_Fast_Cursor_Left",        0xfe2c },
 { "XK_ISO_Fast_Cursor_Right",       0xfe2d },
 { "XK_ISO_Fast_Cursor_Up",          0xfe2e },
 { "XK_ISO_Fast_Cursor_Down",        0xfe2f },
 { "XK_ISO_Continuous_Underline",    0xfe30 },
 { "XK_ISO_Discontinuous_Underline", 0xfe31 },
 { "XK_ISO_Emphasize",               0xfe32 },
 { "XK_ISO_Center_Object",           0xfe33 },
 { "XK_ISO_Enter",                   0xfe34 },

/* { "Unmapped",                       0xfe35 - 0xfe4f }, */

 { "XK_dead_grave",                  0xfe50 },
 { "XK_dead_acute",                  0xfe51 },
 { "XK_dead_circumflex",             0xfe52 },
 { "XK_dead_tilde",                  0xfe53 },
/* { "XK_dead_perispomeni",            0xfe53 },   alias for dead_tilde */
 { "XK_dead_macron",                 0xfe54 },
 { "XK_dead_breve",                  0xfe55 },
 { "XK_dead_abovedot",               0xfe56 },
 { "XK_dead_diaeresis",              0xfe57 },
 { "XK_dead_abovering",              0xfe58 },
 { "XK_dead_doubleacute",            0xfe59 },
 { "XK_dead_caron",                  0xfe5a },
 { "XK_dead_cedilla",                0xfe5b },
 { "XK_dead_ogonek",                 0xfe5c },
 { "XK_dead_iota",                   0xfe5d },
 { "XK_dead_voiced_sound",           0xfe5e },
 { "XK_dead_semivoiced_sound",       0xfe5f },
 { "XK_dead_belowdot",               0xfe60 },
 { "XK_dead_hook",                   0xfe61 },
 { "XK_dead_horn",                   0xfe62 },
 { "XK_dead_stroke",                 0xfe63 },
 { "XK_dead_abovecomma",             0xfe64 },
/*  { "XK_dead_psili",                  0xfe64 },  alias for dead_abovecomma */
 { "XK_dead_abovereversedcomma",     0xfe65 },
/*  { "XK_dead_dasia",                  0xfe65 },  alias for dead_abovereversedcomma */
 { "XK_dead_doublegrave",            0xfe66 },
 { "XK_dead_belowring",              0xfe67 },
 { "XK_dead_belowmacron",            0xfe68 },
 { "XK_dead_belowcircumflex",        0xfe69 },
 { "XK_dead_belowtilde",             0xfe6a },
 { "XK_dead_belowbreve",             0xfe6b },
 { "XK_dead_belowdiaeresis",         0xfe6c },
 { "XK_dead_invertedbreve",          0xfe6d },
 { "XK_dead_belowcomma",             0xfe6e },
 { "XK_dead_currency",               0xfe6f },

 { "XK_AccessX_Enable",              0xfe70 },
 { "XK_AccessX_Feedback_Enable",     0xfe71 },
 { "XK_RepeatKeys_Enable",           0xfe72 },
 { "XK_SlowKeys_Enable",             0xfe73 },
 { "XK_BounceKeys_Enable",           0xfe74 },
 { "XK_StickyKeys_Enable",           0xfe75 },
 { "XK_MouseKeys_Enable",            0xfe76 },
 { "XK_MouseKeys_Accel_Enable",      0xfe77 },
 { "XK_Overlay1_Enable",             0xfe78 },
 { "XK_Overlay2_Enable",             0xfe79 },
 { "XK_AudibleBell_Enable",          0xfe7a },

 { "Unmapped",                       0xfe7b },
 { "Unmapped",                       0xfe7c },
 { "Unmapped",                       0xfe7d },
 { "Unmapped",                       0xfe7e },
 { "Unmapped",                       0xfe7f },

/* dead vowels for universal syllable entry */
 { "XK_dead_a",                      0xfe80 },
 { "XK_dead_A",                      0xfe81 },
 { "XK_dead_e",                      0xfe82 },
 { "XK_dead_E",                      0xfe83 },
 { "XK_dead_i",                      0xfe84 },
 { "XK_dead_I",                      0xfe85 },
 { "XK_dead_o",                      0xfe86 },
 { "XK_dead_O",                      0xfe87 },
 { "XK_dead_u",                      0xfe88 },
 { "XK_dead_U",                      0xfe89 },
 { "XK_dead_small_schwa",            0xfe8a },
 { "XK_dead_capital_schwa",          0xfe8b },
 { "XK_dead_greek",                  0xfe8c },

 { "Unmapped",                       0xfe8d },
 { "Unmapped",                       0xfe8e },
 { "Unmapped",                       0xfe8f },

/* extra dead elements for German T3 layout */
 { "XK_dead_lowline",                0xfe90 },
 { "XK_dead_aboveverticalline",      0xfe91 },
 { "XK_dead_belowverticalline",      0xfe92 },
 { "XK_dead_longsolidusoverlay",     0xfe93 },

 { "Unmapped",                       0xfe94 },
 { "Unmapped",                       0xfe95 },
 { "Unmapped",                       0xfe96 },
 { "Unmapped",                       0xfe97 },
 { "Unmapped",                       0xfe98 },
 { "Unmapped",                       0xfe99 },
 { "Unmapped",                       0xfe9a },
 { "Unmapped",                       0xfe9b },
 { "Unmapped",                       0xfe9c },
 { "Unmapped",                       0xfe9d },
 { "Unmapped",                       0xfe9e },
 { "Unmapped",                       0xfe9f },

/* Single-Stroke Multiple-Character N-Graph Keysyms For The X Input Method */

 { "XK_ch",                          0xfea0 },
 { "XK_Ch",                          0xfea1 },
 { "XK_CH",                          0xfea2 },
 { "XK_c_h",                         0xfea3 },
 { "XK_C_h",                         0xfea4 },
 { "XK_C_H",                         0xfea5 },

/* { "Unmapped",                       0xfea6 - 0xfecf }, */

 { "XK_First_Virtual_Screen",        0xfed0 },
 { "XK_Prev_Virtual_Screen",         0xfed1 },
 { "XK_Next_Virtual_Screen",         0xfed2 },
 { "Unmapped",                       0xfed3 },
 { "XK_Last_Virtual_Screen",         0xfed4 },
 { "XK_Terminate_Server",            0xfed5 },

 { "Unmapped",                       0xfed6 },
 { "Unmapped",                       0xfed7 },
 { "Unmapped",                       0xfed8 },
 { "Unmapped",                       0xfed9 },
 { "Unmapped",                       0xfeda },
 { "Unmapped",                       0xfedb },
 { "Unmapped",                       0xfedc },
 { "Unmapped",                       0xfedd },
 { "Unmapped",                       0xfede },
 { "Unmapped",                       0xfedf },

 { "XK_Pointer_Left",                0xfee0 },
 { "XK_Pointer_Right",               0xfee1 },
 { "XK_Pointer_Up",                  0xfee2 },
 { "XK_Pointer_Down",                0xfee3 },
 { "XK_Pointer_UpLeft",              0xfee4 },
 { "XK_Pointer_UpRight",             0xfee5 },
 { "XK_Pointer_DownLeft",            0xfee6 },
 { "XK_Pointer_DownRight",           0xfee7 },
 { "XK_Pointer_Button_Dflt",         0xfee8 },
 { "XK_Pointer_Button1",             0xfee9 },
 { "XK_Pointer_Button2",             0xfeea },
 { "XK_Pointer_Button3",             0xfeeb },
 { "XK_Pointer_Button4",             0xfeec },
 { "XK_Pointer_Button5",             0xfeed },
 { "XK_Pointer_DblClick_Dflt",       0xfeee },
 { "XK_Pointer_DblClick1",           0xfeef },
 { "XK_Pointer_DblClick2",           0xfef0 },
 { "XK_Pointer_DblClick3",           0xfef1 },
 { "XK_Pointer_DblClick4",           0xfef2 },
 { "XK_Pointer_DblClick5",           0xfef3 },
 { "XK_Pointer_Drag_Dflt",           0xfef4 },
 { "XK_Pointer_Drag1",               0xfef5 },
 { "XK_Pointer_Drag2",               0xfef6 },
 { "XK_Pointer_Drag3",               0xfef7 },
 { "XK_Pointer_Drag4",               0xfef8 },
 { "XK_Pointer_EnableKeys",          0xfef9 },
 { "XK_Pointer_Accelerate",          0xfefa },
 { "XK_Pointer_DfltBtnNext",         0xfefb },
 { "XK_Pointer_DfltBtnPrev",         0xfefc },
 { "XK_Pointer_Drag5",               0xfefd },

 { "Unmapped",                       0xfefe },
 { "Unmapped",                       0xfeff },
 { "Unmapped",                       0xff00 },
 { "Unmapped",                       0xff01 },
 { "Unmapped",                       0xff02 },
 { "Unmapped",                       0xff03 },
 { "Unmapped",                       0xff04 },
 { "Unmapped",                       0xff05 },
 { "Unmapped",                       0xff06 },
 { "Unmapped",                       0xff07 },

 { "XK_BackSpace",                   0xff08 },  /* Back space, back char */
 { "XK_Tab",                         0xff09 },
 { "XK_Linefeed",                    0xff0a },  /* Linefeed, LF */
 { "XK_Clear",                       0xff0b },
 { "Unmapped",                       0xff0c },
 { "XK_Return",                      0xff0d },  /* Return, enter */

 { "Unmapped",                       0xff0e },
 { "Unmapped",                       0xff0f },
 { "Unmapped",                       0xff10 },
 { "Unmapped",                       0xff11 },
 { "Unmapped",                       0xff12 },

 { "XK_Pause",                       0xff13 },  /* Pause, hold */
 { "XK_Scroll_Lock",                 0xff14 },
 { "XK_Sys_Req",                     0xff15 },

 { "Unmapped",                       0xff16 },
 { "Unmapped",                       0xff17 },
 { "Unmapped",                       0xff18 },
 { "Unmapped",                       0xff19 },
 { "Unmapped",                       0xff1a },

 { "XK_Escape",                      0xff1b },

/* International & multi-key character composition */

 { "Unmapped",                       0xff1c },
 { "Unmapped",                       0xff1d },
 { "Unmapped",                       0xff1e },
 { "Unmapped",                       0xff1f },

 { "XK_Multi_key",                   0xff20 },  /* Multi-key character compose */

/* Japanese keyboard support */

 { "XK_Kanji",                       0xff21 },  /* Kanji, Kanji convert */
 { "XK_Muhenkan",                    0xff22 },  /* Cancel Conversion */
 { "XK_Henkan_Mode",                 0xff23 },  /* Start/Stop Conversion */
/* { "XK_Henkan",                      0xff23 },   Alias for Henkan_Mode */
 { "XK_Romaji",                      0xff24 },  /* to Romaji */
 { "XK_Hiragana",                    0xff25 },  /* to Hiragana */
 { "XK_Katakana",                    0xff26 },  /* to Katakana */
 { "XK_Hiragana_Katakana",           0xff27 },  /* Hiragana/Katakana toggle */
 { "XK_Zenkaku",                     0xff28 },  /* to Zenkaku */
 { "XK_Hankaku",                     0xff29 },  /* to Hankaku */
 { "XK_Zenkaku_Hankaku",             0xff2a },  /* Zenkaku/Hankaku toggle */
 { "XK_Touroku",                     0xff2b },  /* Add to Dictionary */
 { "XK_Massyo",                      0xff2c },  /* Delete from Dictionary */
 { "XK_Kana_Lock",                   0xff2d },  /* Kana Lock */
 { "XK_Kana_Shift",                  0xff2e },  /* Kana Shift */
 { "XK_Eisu_Shift",                  0xff2f },  /* Alphanumeric Shift */
 { "XK_Eisu_toggle",                 0xff30 },  /* Alphanumeric toggle */

 { "Unmapped",                       0xff31 },
 { "Unmapped",                       0xff32 },
 { "Unmapped",                       0xff33 },
 { "Unmapped",                       0xff34 },
 { "Unmapped",                       0xff35 },
 { "Unmapped",                       0xff36 },

 { "XK_Codeinput",                   0xff37 },
/* { "XK_Kanji_Bangou",                0xff37 },   Codeinput */

 { "Unmapped",                       0xff38 },
 { "Unmapped",                       0xff39 },
 { "Unmapped",                       0xff3a },
 { "Unmapped",                       0xff3b },

 { "XK_SingleCandidate",             0xff3c },
 { "XK_MultipleCandidate",           0xff3d },
/* { "XK_Zen_Koho",                    0xff3d },   Multiple/All Candidate(s) */
 { "XK_PreviousCandidate",           0xff3e },
/* { "XK_Mae_Koho",                    0xff3e },   Previous Candidate */

/* 0xff31 thru 0xff3f are under XK_KOREAN */

 { "Unmapped",                       0xff3f },
 { "Unmapped",                       0xff40 },
 { "Unmapped",                       0xff41 },
 { "Unmapped",                       0xff42 },
 { "Unmapped",                       0xff43 },
 { "Unmapped",                       0xff44 },
 { "Unmapped",                       0xff45 },
 { "Unmapped",                       0xff46 },
 { "Unmapped",                       0xff47 },
 { "Unmapped",                       0xff48 },
 { "Unmapped",                       0xff49 },
 { "Unmapped",                       0xff4a },
 { "Unmapped",                       0xff4b },
 { "Unmapped",                       0xff4c },
 { "Unmapped",                       0xff4d },
 { "Unmapped",                       0xff4e },
 { "Unmapped",                       0xff4f },

/* Cursor control & motion */

 { "XK_Home",                        0xff50 },
 { "XK_Left",                        0xff51 },  /* Move left, left arrow */
 { "XK_Up",                          0xff52 },  /* Move up, up arrow */
 { "XK_Right",                       0xff53 },  /* Move right, right arrow */
 { "XK_Down",                        0xff54 },  /* Move down, down arrow */
 { "XK_Page_Up",                     0xff55 },
/* { "XK_Prior",                       0xff55 },   Prior, previous */
 { "XK_Page_Down",                   0xff56 },
/* { "XK_Next",                        0xff56 },   Next */
 { "XK_End",                         0xff57 },  /* EOL */
 { "XK_Begin",                       0xff58 },  /* BOL */

 { "Unmapped",                       0xff59 },
 { "Unmapped",                       0xff5a },
 { "Unmapped",                       0xff5b },
 { "Unmapped",                       0xff5c },
 { "Unmapped",                       0xff5d },
 { "Unmapped",                       0xff5e },
 { "Unmapped",                       0xff5f },

/* Misc functions */

 { "XK_Select",                      0xff60 },  /* Select, mark */
 { "XK_Print",                       0xff61 },
 { "XK_Execute",                     0xff62 },  /* Execute, run, do */
 { "XK_Insert",                      0xff63 },  /* Insert, insert here */
 { "Unmapped",                       0xff64 },
 { "XK_Undo",                        0xff65 },
 { "XK_Redo",                        0xff66 },  /* Redo, again */
 { "XK_Menu",                        0xff67 },
 { "XK_Find",                        0xff68 },  /* Find, search */
 { "XK_Cancel",                      0xff69 },  /* Cancel, stop, abort, exit */
 { "XK_Help",                        0xff6a },  /* Help */
 { "XK_Break",                       0xff6b },

 { "Unmapped",                       0xff6c },
 { "Unmapped",                       0xff6d },
 { "Unmapped",                       0xff6e },
 { "Unmapped",                       0xff6f },
 { "Unmapped",                       0xff70 },
 { "Unmapped",                       0xff71 },
 { "Unmapped",                       0xff72 },
 { "Unmapped",                       0xff73 },
 { "Unmapped",                       0xff74 },
 { "Unmapped",                       0xff75 },
 { "Unmapped",                       0xff76 },
 { "Unmapped",                       0xff77 },
 { "Unmapped",                       0xff78 },
 { "Unmapped",                       0xff79 },
 { "Unmapped",                       0xff7a },
 { "Unmapped",                       0xff7b },
 { "Unmapped",                       0xff7c },
 { "Unmapped",                       0xff7d },

 { "XK_Mode_switch",                 0xff7e },  /* Character set switch */
/* { "XK_script_switch",               0xff7e },   Alias for mode_switch */
/* { "XK_ISO_Group_Shift",             0xff7e },   Alias for mode_switch */
 { "XK_Num_Lock",                    0xff7f },

/* Keypad functions, keypad numbers cleverly chosen to map to ASCII */

 { "XK_KP_Space",                    0xff80 },  /* Space */

 { "Unmapped",                       0xff81 },
 { "Unmapped",                       0xff82 },
 { "Unmapped",                       0xff83 },
 { "Unmapped",                       0xff84 },
 { "Unmapped",                       0xff85 },
 { "Unmapped",                       0xff86 },
 { "Unmapped",                       0xff87 },
 { "Unmapped",                       0xff88 },

 { "XK_KP_Tab",                      0xff89 },

 { "Unmapped",                       0xff8a },
 { "Unmapped",                       0xff8b },
 { "Unmapped",                       0xff8c },

 { "XK_KP_Enter",                    0xff8d },  /* Enter */

 { "Unmapped",                       0xff8e },
 { "Unmapped",                       0xff8f },
 { "Unmapped",                       0xff90 },

 { "XK_KP_F1",                       0xff91 },  /* PF1, KP_A, ... */
 { "XK_KP_F2",                       0xff92 },
 { "XK_KP_F3",                       0xff93 },
 { "XK_KP_F4",                       0xff94 },
 { "XK_KP_Home",                     0xff95 },
 { "XK_KP_Left",                     0xff96 },
 { "XK_KP_Up",                       0xff97 },
 { "XK_KP_Right",                    0xff98 },
 { "XK_KP_Down",                     0xff99 },
 { "XK_KP_Page_Up",                  0xff9a },
/* { "XK_KP_Prior",                    0xff9a }, */
 { "XK_KP_Page_Down",                0xff9b },
/* { "XK_KP_Next",                     0xff9b }, */
 { "XK_KP_End",                      0xff9c },
 { "XK_KP_Begin",                    0xff9d },
 { "XK_KP_Insert",                   0xff9e },
 { "XK_KP_Delete",                   0xff9f },

 { "Unmapped",                       0xffa0 },
 { "Unmapped",                       0xffa1 },
 { "Unmapped",                       0xffa2 },
 { "Unmapped",                       0xffa3 },
 { "Unmapped",                       0xffa4 },
 { "Unmapped",                       0xffa5 },
 { "Unmapped",                       0xffa6 },
 { "Unmapped",                       0xffa7 },
 { "Unmapped",                       0xffa8 },
 { "Unmapped",                       0xffa9 },

 { "XK_KP_Multiply",                 0xffaa },
 { "XK_KP_Add",                      0xffab },
 { "XK_KP_Separator",                0xffac },  /* Separator, often comma */
 { "XK_KP_Subtract",                 0xffad },
 { "XK_KP_Decimal",                  0xffae },
 { "XK_KP_Divide",                   0xffaf },
 { "XK_KP_0",                        0xffb0 },
 { "XK_KP_1",                        0xffb1 },
 { "XK_KP_2",                        0xffb2 },
 { "XK_KP_3",                        0xffb3 },
 { "XK_KP_4",                        0xffb4 },
 { "XK_KP_5",                        0xffb5 },
 { "XK_KP_6",                        0xffb6 },
 { "XK_KP_7",                        0xffb7 },
 { "XK_KP_8",                        0xffb8 },
 { "XK_KP_9",                        0xffb9 },

 { "Unmapped",                       0xffba },
 { "Unmapped",                       0xffbb },
 { "Unmapped",                       0xffbc },

 { "XK_KP_Equal",                    0xffbd },  /* Equals */

/*
 * Auxiliary functions; note the duplicate definitions for left and right
 * function keys;  Sun keyboards and a few other manufacturers have such
 * function key groups on the left and/or right sides of the keyboard.
 * We've not found a keyboard with more than 35 function keys total.
 */

 { "XK_F1",                          0xffbe },
 { "XK_F2",                          0xffbf },
 { "XK_F3",                          0xffc0 },
 { "XK_F4",                          0xffc1 },
 { "XK_F5",                          0xffc2 },
 { "XK_F6",                          0xffc3 },
 { "XK_F7",                          0xffc4 },
 { "XK_F8",                          0xffc5 },
 { "XK_F9",                          0xffc6 },
 { "XK_F10",                         0xffc7 },
 { "XK_F11",                         0xffc8 },
/* { "XK_L1",                          0xffc8 }, */
 { "XK_F12",                         0xffc9 },
/* { "XK_L2",                          0xffc9 }, */
 { "XK_F13",                         0xffca },
/* { "XK_L3",                          0xffca }, */
 { "XK_F14",                         0xffcb },
/* { "XK_L4",                          0xffcb }, */
 { "XK_F15",                         0xffcc },
/* { "XK_L5",                          0xffcc }, */
 { "XK_F16",                         0xffcd },
/* { "XK_L6",                          0xffcd }, */
 { "XK_F17",                         0xffce },
/* { "XK_L7",                          0xffce }, */
 { "XK_F18",                         0xffcf },
/* { "XK_L8",                          0xffcf }, */
 { "XK_F19",                         0xffd0 },
/* { "XK_L9",                          0xffd0 }, */
 { "XK_F20",                         0xffd1 },
/* { "XK_L10",                         0xffd1 }, */
 { "XK_F21",                         0xffd2 },
/* { "XK_R1",                          0xffd2 }, */
 { "XK_F22",                         0xffd3 },
/* { "XK_R2",                          0xffd3 }, */
 { "XK_F23",                         0xffd4 },
/* { "XK_R3",                          0xffd4 }, */
 { "XK_F24",                         0xffd5 },
/* { "XK_R4",                          0xffd5 }, */
 { "XK_F25",                         0xffd6 },
/* { "XK_R5",                          0xffd6 }, */
 { "XK_F26",                         0xffd7 },
/* { "XK_R6",                          0xffd7 }, */
 { "XK_F27",                         0xffd8 },
/* { "XK_R7",                          0xffd8 }, */
 { "XK_F28",                         0xffd9 },
/* { "XK_R8",                          0xffd9 }, */
 { "XK_F29",                         0xffda },
/* { "XK_R9",                          0xffda }, */
 { "XK_F30",                         0xffdb },
/* { "XK_R10",                         0xffdb }, */
 { "XK_F31",                         0xffdc },
/* { "XK_R11",                         0xffdc }, */
 { "XK_F32",                         0xffdd },
/* { "XK_R12",                         0xffdd }, */
 { "XK_F33",                         0xffde },
/* { "XK_R13",                         0xffde }, */
 { "XK_F34",                         0xffdf },
/* { "XK_R14",                         0xffdf }, */
 { "XK_F35",                         0xffe0 },
/* { "XK_R15",                         0xffe0 }, */

/* Modifiers */

 { "XK_Shift_L",                     0xffe1 },  /* Left shift */
 { "XK_Shift_R",                     0xffe2 },  /* Right shift */
 { "XK_Control_L",                   0xffe3 },  /* Left control */
 { "XK_Control_R",                   0xffe4 },  /* Right control */
 { "XK_Caps_Lock",                   0xffe5 },  /* Caps lock */
 { "XK_Shift_Lock",                  0xffe6 },  /* Shift lock */

 { "XK_Meta_L",                      0xffe7 },  /* Left meta */
 { "XK_Meta_R",                      0xffe8 },  /* Right meta */
 { "XK_Alt_L",                       0xffe9 },  /* Left alt */
 { "XK_Alt_R",                       0xffea },  /* Right alt */
 { "XK_Super_L",                     0xffeb },  /* Left super */
 { "XK_Super_R",                     0xffec },  /* Right super */
 { "XK_Hyper_L",                     0xffed },  /* Left hyper */
 { "XK_Hyper_R",                     0xffee },  /* Right hyper */

 { "Unmapped",                       0xffef },
 { "Unmapped",                       0xfff0 },
 { "Unmapped",                       0xfff1 },
 { "Unmapped",                       0xfff2 },
 { "Unmapped",                       0xfff3 },
 { "Unmapped",                       0xfff4 },
 { "Unmapped",                       0xfff5 },
 { "Unmapped",                       0xfff6 },
 { "Unmapped",                       0xfff7 },
 { "Unmapped",                       0xfff8 },
 { "Unmapped",                       0xfff9 },
 { "Unmapped",                       0xfffa },
 { "Unmapped",                       0xfffb },
 { "Unmapped",                       0xfffc },
 { "Unmapped",                       0xfffd },
 { "Unmapped",                       0xfffe },

 { "XK_Delete",                      0xffff }  /* Delete, rubout */

};
