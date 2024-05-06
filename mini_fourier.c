#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "translators/disk_bitmap.h"
#include "translators/bitmap_pointslist.h"
#include "translators/shortcycle.h"
#include "translators/pointslist_doubleslist.h"
#include "translators/doubleslist_fourier.h"
#include "translators/homothetie.h"
#include "types/fbase.h"

struct args_state {
    const char *source;
    const char *dest_prefix;
    double (*base)(size_t,double);
    size_t starting_mode;
    size_t mode_increment;
    size_t mode_quad;
    size_t pictures;
    double xscale;
    double xshift;
    double yscale;
    double yshift;
    unsigned int starting_mode_set:1;
    unsigned int mode_increment_set:1;
    unsigned int mode_quad_set:1;
    unsigned int pictures_set:1;
    unsigned int xscale_set:1;
    unsigned int xshift_set:1;
    unsigned int yscale_set:1;
    unsigned int yshift_set:1;
    unsigned int help_set:1;
};

static int parse_source(const char *arg, struct args_state *state) {
    if (state->source != NULL) {
        dprintf(2, "Source image is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (source)\n");
        return -1;
    }
    state->source = arg;
    return 0;
}

static int parse_dest_prefix(const char *arg, struct args_state *state) {
    if (state->dest_prefix != NULL) {
        dprintf(2, "Destination prefix image is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (destination_prefix)\n");
        return -1;
    }
    state->dest_prefix = arg;
    return 0;
}

static int parse_base(const char *arg, struct args_state *state) {
    if (state->base != NULL) {
        dprintf(2, "Base is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (base)\n");
        return -1;
    }
    if (strcmp(arg, "fourier") == 0) {
        state->base = fourier;
    } else if (strcmp(arg, "heaviside") == 0) {
        state->base = heaviside;
    } else if (strcmp(arg, "legendre") == 0) {
        state->base = legendre;
    } else {
        dprintf(2, "Provided base is not supported (try \"fourier\", \"legendre\" or \"heaviside\")\n");
        return -1;
    }
    return 0;
}

static int parse_starting_mode(const char *arg, struct args_state *state) {
    if (state->starting_mode_set) {
        dprintf(2, "Starting mode is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (starting_mode)\n");
        return -1;
    }
    char *end = NULL;
    state->starting_mode = strtoull(arg, &end, 0);
    if (*end != '\0') {
        dprintf(2, "Cannot parse starting harmonic mode\n");
        return -1;
    }
    state->starting_mode_set = 1;
    return 0;
}

static int parse_mode_increment(const char *arg, struct args_state *state) {
    if (state->mode_increment_set) {
        dprintf(2, "Mode increment is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (mode_increment)\n");
        return -1;
    }
    char *end = NULL;
    state->mode_increment = strtoull(arg, &end, 0);
    if (*end != '\0') {
        dprintf(2, "Cannot parse harmonic mode increment\n");
        return -1;
    }
    state->mode_increment_set = 1;
    return 0;
}

static int parse_mode_quad(const char *arg, struct args_state *state) {
    if (state->mode_quad_set) {
        dprintf(2, "Mode quad increment is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (mode_quad)\n");
        return -1;
    }
    char *end = NULL;
    state->mode_quad = strtoull(arg, &end, 0);
    if (*end != '\0') {
        dprintf(2, "Cannot parse harmonic mode quad increment\n");
        return -1;
    }
    state->mode_quad_set = 1;
    return 0;
}

static int parse_pictures(const char *arg, struct args_state *state) {
    if (state->pictures_set) {
        dprintf(2, "Pictures is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (pictures)\n");
        return -1;
    }
    char *end = NULL;
    state->pictures = strtoull(arg, &end, 0);
    if (*end != '\0') {
        dprintf(2, "Cannot parse number of pictures\n");
        return -1;
    }
    state->pictures_set = 1;
    return 0;
}

static int parse_xscale(const char *arg, struct args_state *state) {
    if (state->xscale_set) {
        dprintf(2, "Scale is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (scale)\n");
        return -1;
    }
    char *end = NULL;
    state->xscale = strtod(arg, &end);
    if (*end != '\0') {
        dprintf(2, "Cannot parse scale\n");
        return -1;
    }
    state->xscale_set = 1;
    return 0;
}

static int parse_xshift(const char *arg, struct args_state *state) {
    if (state->xshift_set) {
        dprintf(2, "Shift is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (shift)\n");
        return -1;
    }
    char *end = NULL;
    state->xshift = strtod(arg, &end);
    if (*end != '\0') {
        dprintf(2, "Cannot parse shift\n");
        return -1;
    }
    state->xshift_set = 1;
    return 0;
}

static int parse_yscale(const char *arg, struct args_state *state) {
    if (state->yscale_set) {
        dprintf(2, "Scale is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (scale)\n");
        return -1;
    }
    char *end = NULL;
    state->yscale = strtod(arg, &end);
    if (*end != '\0') {
        dprintf(2, "Cannot parse scale\n");
        return -1;
    }
    state->yscale_set = 1;
    return 0;
}

static int parse_yshift(const char *arg, struct args_state *state) {
    if (state->yshift_set) {
        dprintf(2, "Shift is already set\n");
        return -1;
    }
    if (arg == NULL) {
        dprintf(2, "Missing parameter (shift)\n");
        return -1;
    }
    char *end = NULL;
    state->yshift = strtod(arg, &end);
    if (*end != '\0') {
        dprintf(2, "Cannot parse shift\n");
        return -1;
    }
    state->yshift_set = 1;
    return 0;
}

static int parse_help(const char *arg, struct args_state *state) {
    if (state->help_set) {
        dprintf(2, "Help is already set\n");
        return -1;
    }
    if (arg != NULL) {
        dprintf(2, "Unexpected parameter (help)\n");
        return -1;
    }
    state->help_set = 1;
    return 0;
}

struct option {
    const char *arg_name;
    const char *parameter_name;
    const char *description;
    const char *deflt;
    int (*parse)(const char *arg, struct args_state *state);
};

static struct option options[] = {
    {
        .arg_name = "source",
        .parameter_name = "file_name",
        .description = "<file_name> must be a non-compressed bitmap file with data to proces",
        .parse = parse_source,
        .deflt = NULL,
    },
    {
        .arg_name = "destination_prefix",
        .parameter_name = "file_name_prefix",
        .description = "<file_name_prefix> must be a prefix for all generated files",
        .parse = parse_dest_prefix,
        .deflt = "<file_name> with ending \".bmp\" stripped",
    },
    {
        .arg_name = "base",
        .parameter_name = "orth_fun_base",
        .description = "<orth_fun_base> must be either \"fourier\", \"legendre\" or \"heaviside\"",
        .parse = parse_base,
        .deflt = "fourier",
    },
    {
        .arg_name = "starting_mode",
        .parameter_name = "mode",
        .description = "<mode> is an integer for the first harmonic to be generated",
        .parse = parse_starting_mode,
        .deflt = "0",
    },
    {
        .arg_name = "mode_increment",
        .parameter_name = "delta_mode",
        .description = "<delta_mode> is an integer for the difference between two computed harmonics to be generated",
        .parse = parse_mode_increment,
        .deflt = "1",
    },
    {
        .arg_name = "mode_quad",
        .parameter_name = "ddelta_mode",
        .description = "<ddelta_mode> is an integer for the difference between two computed differences of harmonics to be generated",
        .parse = parse_mode_quad,
        .deflt = "0",
    },
    {
        .arg_name = "pictures",
        .parameter_name = "n",
        .description = "<n> is the number of pictures to generate",
        .parse = parse_pictures,
        .deflt = "1",
    },
    {
        .arg_name = "xscale",
        .parameter_name = "kx",
        .description = "<kx> is the scale to apply on generated picture (zoom-in/zoom-out)",
        .parse = parse_xscale,
        .deflt = "1.0",
    },
    {
        .arg_name = "xshift",
        .parameter_name = "ox",
        .description = "<ox> is the offset to apply on generated picture (zoom-in/zoom-out)",
        .parse = parse_xshift,
        .deflt = "0.0",
    },
    {
        .arg_name = "yscale",
        .parameter_name = "ky",
        .description = "<ky> is the scale to apply on generated picture (zoom-in/zoom-out)",
        .parse = parse_yscale,
        .deflt = "1.0",
    },
    {
        .arg_name = "yshift",
        .parameter_name = "oy",
        .description = "<oy> is the offset to apply on generated picture (zoom-in/zoom-out)",
        .parse = parse_yshift,
        .deflt = "0.0",
    },
    {
        .arg_name = "help",
        .parameter_name = NULL,
        .description = "Prints this help",
        .parse = parse_help,
        .deflt = NULL,
    },
    { 0 }
};


static void show_help(const char *cmd) {
    dprintf(2, "%s ", cmd);
    struct option *opt;
    opt = options;
    while (opt->arg_name != NULL) {
        _Bool mandatory = (opt->parameter_name != NULL) && (opt->deflt == NULL);
        if (!mandatory) {
            dprintf(2, "[");
        }
        dprintf(2, "--%s", opt->arg_name);
        if (opt->parameter_name != NULL) {
            dprintf(2, " <%s>", opt->parameter_name);
        }
        if (!mandatory) {
            dprintf(2, "]");
        }
        dprintf(2, " ");
        ++opt;
    }
    dprintf(2, "\n");
    opt = options;
    while (opt->arg_name != NULL) {
        dprintf(2, "  --%s", opt->arg_name);
        if (opt->parameter_name != NULL) {
            dprintf(2, " <%s>", opt->parameter_name);
        }
        dprintf(2, ": %s", opt->description);
        if (opt->deflt != NULL) {
            dprintf(2, " (deflt is \"%s\")", opt->deflt);
        }
        dprintf(2, "\n");
        ++opt;
    }
    return;
}

static int parse_args(struct args_state *args, int argc, char **argv) {
    int i = 1;
    while (i < argc) {
        if ((argv[i][0] == '\0') || (argv[i][1] == '\0')) {
            dprintf(2, "%s: invalid argument\n", argv[i]);
            return -1;
        }
        if ((argv[i][0] != '-') || (argv[i][1] != '-')) {
            dprintf(2, "%s: invalid argument\n", argv[i]);
            return -1;
        }
        struct option *opt = options;
        while (opt->arg_name != NULL) {
            if (strcmp(argv[i] + 2, opt->arg_name) == 0) {
                break;
            }
            ++opt;
        }
        if (opt->arg_name == NULL) {
            dprintf(2, "%s: invalid argument\n", argv[i]);
            return -1;
        }
        const char *param = NULL;
        if (opt->parameter_name != NULL) {
            ++i;
            if (i >= argc) {
                dprintf(2, "%s: missing parameter\n", argv[i-1]);
                return -1;
            }
            param = argv[i];
        }
        int r = opt->parse(param, args);
        if (r != 0) {
            return -1;
        }
        ++i;
    }
    return 0;
}

static int set_deflts(struct args_state *args) {
    if (args->source == NULL) {
        dprintf(2, "Missing source\n");
        return -1;
    }
    if (args->dest_prefix == NULL) {
        args->dest_prefix = args->source;
    }
    if (args->base == NULL) {
        args->base = fourier;
    }
    if (args->starting_mode_set == 0) {
        args->starting_mode = 0;
        args->starting_mode_set = 1;
    }
    if (args->mode_increment_set == 0) {
        args->mode_increment = 1;
        args->mode_increment_set = 1;
    }
    if (args->mode_quad_set == 0) {
        args->mode_quad = 0;
        args->mode_quad_set = 1;
    }
    if (args->pictures_set == 0) {
        args->pictures = 1;
        args->pictures_set = 1;
    }
    if (args->xscale_set == 0) {
        args->xscale = 1.0;
        args->xscale_set = 1;
    }
    if (args->xshift_set == 0) {
        args->xshift = 0.0;
        args->xshift_set = 1;
    }
    if (args->yscale_set == 0) {
        args->yscale = 1.0;
        args->yscale_set = 1;
    }
    if (args->yshift_set == 0) {
        args->yshift = 0.0;
        args->yshift_set = 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    struct args_state args = { 0 };
    int r;
    r = parse_args(&args, argc, argv);
    if (r != 0) {
        args.help_set = 1;
    }
    r = set_deflts(&args);
    if (r != 0) {
        args.help_set = 1;
    }

    if (args.help_set) {
        show_help(argv[0]);
        return -1;
    }
    size_t last_mode = args.pictures * (args.pictures * args.mode_quad + args.mode_increment) + args.starting_mode;
    if (last_mode > 999999) {
        dprintf(2, "Too many modes\n");
        return -1;
    }
    static char file_name[256];
    size_t len = strlen(args.source);
    if (len < 4) {
        dprintf(2, "File name is too short\n");
        return -1;
    }
    if (strcmp(args.source + len - 4, ".bmp") != 0) {
        dprintf(2, "File extension is not .bmp\n");
        return -1;
    }
    if ((strlen(args.source) + 7) >= sizeof(file_name)) {
        dprintf(2, "File name is too long\n");
        return -1;
    }
    if (args.mode_increment == 0) {
        dprintf(2, "The mode increment must be at least 1\n");
        return -1;
    }
    struct raw_bitmap *bm0 = disk_to_bitmap(args.source);
    if (bm0 == NULL) {
        dprintf(2, "Failed to load bitmap image\n");
        return -1;
    }
    struct raw_bitmap_info rbi = get_raw_bitmap_info(bm0);
    dprintf(2, "Successfully loaded the bitmap image\n");

    struct points_list *pl0 = get_points_list(bm0, 1);
    destroy_raw_bitmap(bm0);
    if (pl0 == NULL) {
        dprintf(2, "Could not extract the list of points from the bitmap\n");
        return -1;
    }
    dprintf(2, "Extracted list of points\n");

    struct points_list *pl1 = short_cycle(pl0);
    destroy_points_list(pl0);
    if (pl1 == NULL) {
        dprintf(2, "Could not compute a cycle for drawings\n");
        return -1;
    }
    size_t cycle_length = get_points_num(pl1);
    dprintf(2, "Cycle is computed\n");

    struct split sp = split_points_list(pl1, rbi.width, rbi.height);
    destroy_points_list(pl1);
    if (sp.dlx == NULL) {
        destroy_doubles_list(sp.dly);
        dprintf(2, "Cannot extract the X sequence\n");
        return -1;
    }
    if (sp.dly == NULL) {
        destroy_doubles_list(sp.dlx);
        dprintf(2, "Cannot extract the Y sequence\n");
        return -1;
    }
    dprintf(2, "X and Y sequences extracted\n");

    struct doubles_list *dlx = homothetie(sp.dlx, args.xscale, args.xshift);
    destroy_doubles_list(sp.dlx);
    sp.dlx = dlx;
    if (sp.dlx == NULL) {
        dprintf(2, "X-Rescaling issue\n");
        destroy_doubles_list(sp.dly);
        return -1;
    }
    dprintf(2, "X coordinates rescaled and shifted\n");

    struct doubles_list *dly = homothetie(sp.dly, args.yscale, args.yshift);
    destroy_doubles_list(sp.dly);
    sp.dly = dly;
    if (sp.dly == NULL) {
        dprintf(2, "Y-Rescaling issue\n");
        destroy_doubles_list(sp.dlx);
        return -1;
    }
    dprintf(2, "Y coordinates rescaled and shifted\n");

    struct doubles_list *sx = create_doubles_list(last_mode + 1);
    if (sx == NULL) {
        dprintf(2, "Cannot create the X doubles_list\n");
        destroy_doubles_list(sp.dlx);
        destroy_doubles_list(sp.dly);
        return -1;
    }

    struct doubles_list *sy = create_doubles_list(last_mode + 1);
    if (sx == NULL) {
        dprintf(2, "Cannot create the Y doubles_list\n");
        destroy_doubles_list(sx);
        destroy_doubles_list(sp.dly);
        return -1;
    }

    for (size_t i = 0; i < (last_mode + 1); ++i) {
        double kx = scalar_product(sp.dlx, args.base, i);
        set_double_from_doubles_list(sx, i, kx);
        double ky = scalar_product(sp.dly, args.base, i);
        set_double_from_doubles_list(sy, i, ky);
    }
    destroy_doubles_list(sp.dlx);
    destroy_doubles_list(sp.dly);

    sp.dlx = create_doubles_list(cycle_length);
    if (sp.dlx == NULL) {
        destroy_doubles_list(sx);
        destroy_doubles_list(sy);
        dprintf(2, "Cannot initialize new X points\n");
        return -1;
    }
    sp.dly = create_doubles_list(cycle_length);
    if (sp.dly == NULL) {
        destroy_doubles_list(sp.dlx);
        destroy_doubles_list(sx);
        destroy_doubles_list(sy);
        dprintf(2, "Cannot initialize new Y points\n");
        return -1;
    }

    int ret = 0;
    size_t omode = 0;
    size_t cmode = args.starting_mode;
    for (size_t k = 0; k < args.pictures; ++k) {
        dprintf(2, "---- iteration %zu ------------\n", k);

        for (size_t u = omode; u <= cmode; ++u) {
            double k;
            k = get_double_from_doubles_list(sx, u);
            add_base_vector(sp.dlx, args.base, u, k);
            k = get_double_from_doubles_list(sy, u);
            add_base_vector(sp.dly, args.base, u, k);
        }

        struct points_list *pl2 = merge_doubles_list(sp, rbi.width, rbi.height);
        if (pl2 == NULL) {
            dprintf(2, "Cannot merge back the doubles_list into a sequence of points\n");
            ret = -1;
            break;
        }
        
        dprintf(2, "Sequence of points rebuilt\n");
        
        struct raw_bitmap *bm1 = create_raw_bitmap(rbi);
        if (bm1 == NULL) {
            dprintf(2, "Cannot create an empty bitmap\n");
            destroy_points_list(pl2);
            ret = -1;
            break;
        }
        struct rgba k0 = {
            .a = 0,
            .r = 0,
            .g = 0,
            .b = 0,
        };
        struct rgba k1 = {
            .b = 255,
            .g = 255,
            .r = 255,
            .a = 0,
        };
        (void)set_color(bm1, 0, k0);
        (void)set_color(bm1, 1, k1);
        dprintf(2, "Canvas prepared\n");
        r = draw_points_list(bm1, pl2, 1);
        destroy_points_list(pl2);
        if (r < 0) {
            dprintf(2, "Could not redraw\n");
            destroy_raw_bitmap(bm1);
            ret = -1;
            break;
        }
        dprintf(2, "Picture redrawn in buffer with the exception of %d points out of %zu which are out of canvas\n", r, cycle_length);

        (void)sprintf(file_name, "%.*s_%06zu.bmp", (int)(len - 4), args.dest_prefix, cmode);
        r = bitmap_to_disk(bm1, file_name);
        destroy_raw_bitmap(bm1);
        if (r != 0) {
            dprintf(2, "Write error\n");
            ret = -1;
            break;
        }
        dprintf(2, "Image fully processed\n");
        omode = cmode + 1;
        cmode += args.mode_increment + k * args.mode_quad;
    }
    if (ret == 0) {
        dprintf(2, "-- DONE --\n");
    }
    destroy_doubles_list(sp.dly);
    destroy_doubles_list(sp.dlx);
    destroy_doubles_list(sx);
    destroy_doubles_list(sy);
    return ret;
}
