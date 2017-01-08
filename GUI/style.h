#include "nuklear.h"

enum guitheme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK};

void set_style(struct nk_context *ctx, enum guitheme theme);