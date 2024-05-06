#include "shortcycle.h"
#include <stdlib.h>
#include <stdio.h>

struct edge {
    uint32_t src;
    uint32_t dst;
    uint32_t square_dist;
};

struct complete_graph {
    size_t points_num;
    size_t edges_num;
    struct edge edges[];
};

static uint16_t delta(uint16_t a, uint16_t b) {
    return (a < b) ? (b - a) : (a - b);
}

static uint32_t sqd(const struct point *pi, const struct point *pj) {
    uint32_t dx = delta(pi->x, pj->x);
    uint32_t dy = delta(pi->y, pj->y);
    return dx * dx + dy * dy;
}

static struct complete_graph *get_complete_graph_(const struct points_list *pl) {
    size_t points_num = get_points_num(pl);
    if (points_num < 1) {
        return NULL;
    }
    size_t edges_num = ((points_num - 1) * points_num) / 2;
    struct complete_graph *cg = malloc(sizeof(*cg) + edges_num * sizeof(struct edge));
    if (cg == NULL) {
        return NULL;
    }
    cg->points_num = points_num;
    cg->edges_num = 0;
    for (size_t i = 0; i < (points_num - 1); ++i) {
        struct point pi;
        get_point_from_points_list(pl, i, &pi);
        for (size_t j = i + 1; j < points_num; ++j) {
            struct point pj;
            get_point_from_points_list(pl, j, &pj);
            uint32_t sd = sqd(&pi, &pj);
            size_t sub = cg->edges_num;
            while (sub > 0) {
                size_t x = (sub - 1) / 2;
                if (cg->edges[x].square_dist < sd) {
                    break;
                }
                cg->edges[sub] = cg->edges[x];
                sub = x;
            }
            cg->edges[sub].src = i;
            cg->edges[sub].dst = j;
            cg->edges[sub].square_dist = sd;
            ++cg->edges_num;
        }
    }
    return cg;
}

static int pop_shortest_edge_(struct complete_graph *cg, struct edge *edg) {
    if (cg->edges_num <= 0) {
        return -1;
    }
    *edg = cg->edges[0];
    --cg->edges_num;
    uint32_t sqd = cg->edges[cg->edges_num].square_dist;
    uint32_t slot = 0;
    uint32_t next0 = 2 * slot + 1;
    uint32_t next1 = next0 + 1;
    while (next1 < cg->edges_num) {
        uint32_t sqd0 = cg->edges[next0].square_dist;
        uint32_t sqd1 = cg->edges[next1].square_dist;
        uint32_t nextmin;
        uint32_t sqmin;
        if (sqd0 < sqd1) {
            nextmin = next0;
            sqmin = sqd0;
        } else {
            nextmin = next1;
            sqmin = sqd1;
        }
        if (sqmin < sqd) {
            cg->edges[slot] = cg->edges[nextmin];
            slot = nextmin;
        } else {
            cg->edges[slot] = cg->edges[cg->edges_num];
            return 0;
        }
        next0 = 2 * slot + 1;
        next1 = next0 + 1;
    }
    if (next0 < cg->edges_num) {
        if (cg->edges[next0].square_dist < sqd) {
            cg->edges[slot] = cg->edges[next0];
            slot = next0;
        } else {
            cg->edges[slot] = cg->edges[cg->edges_num];
            return 0;
        }
    }
    cg->edges[slot] = cg->edges[cg->edges_num];
    return 0;
}

struct step {
    uint32_t ptref;
    uint32_t root;
    uint32_t next;
};

struct cycles {
    size_t points_num;
    size_t steps_num;
    struct step steps[];
};

static void add_edge_to_cycles_(struct cycles *c, const struct edge *edg) {
    uint32_t src_root = edg->src;
    while (c->steps[src_root].root != src_root) {
        src_root = c->steps[src_root].root;
    }
    uint32_t src_aux = edg->src;
    while (c->steps[src_aux].root != src_root) {
        uint32_t tmp = c->steps[src_aux].root;
        c->steps[src_aux].root = src_root;
        src_aux = tmp;
    }
    uint32_t dst_root = edg->dst;
    while (c->steps[dst_root].root != dst_root) {
        dst_root = c->steps[dst_root].root;
    }
    _Bool do_merge = (src_root != dst_root);
    c->steps[dst_root].root = src_root;
    uint32_t dst_aux = edg->dst;
    while (c->steps[dst_aux].root != src_root) {
        uint32_t tmp = c->steps[dst_aux].root;
        c->steps[dst_aux].root = src_root;
        dst_aux = tmp;
    }
    if (!do_merge) {
        return;
    }
    c->steps[c->steps_num] = c->steps[edg->src];
    c->steps[c->steps_num + 1] = c->steps[edg->dst];
    c->steps[edg->src].next = c->steps_num + 1;
    c->steps[edg->dst].next = c->steps_num;
    c->steps_num += 2;
    return;
}

static struct cycles *get_cycle_(struct complete_graph *cg) {
    if (cg->points_num < 1) {
        return NULL;
    }
    size_t steps_num = 3 * cg->points_num - 2;
    struct cycles *cy = malloc(sizeof(*cy) + steps_num * sizeof(struct step));
    if (cy == NULL) {
        return NULL;
    }
    cy->points_num = cg->points_num;
    cy->steps_num = 0;
    while (cy->steps_num < cy->points_num) {
        cy->steps[cy->steps_num].ptref = cy->steps_num;
        cy->steps[cy->steps_num].root = cy->steps_num;
        cy->steps[cy->steps_num].next = cy->steps_num;
        ++cy->steps_num;
    }
    while (cy->steps_num < steps_num) {
        struct edge edg;
        if (pop_shortest_edge_(cg, &edg) == 0) {
            add_edge_to_cycles_(cy, &edg);
        }
    }
    return cy;
}

static struct points_list *link_points_(const struct cycles *cy, const struct points_list *l) {
    if (cy->points_num == 0) {
        return NULL;
    }
    size_t points_num = 2 * cy->points_num - 2;
    struct points_list *res = create_points_list(points_num);
    if (res == NULL) {
        return NULL;
    }
    struct point ptx;
    get_point_from_points_list(l, cy->steps[0].ptref, &ptx);
    uint32_t lsqd = 0;
    uint32_t slot = 0;
    while (points_num > 0) {
        --points_num;
        uint32_t ref = cy->steps[slot].ptref;
        struct point pt;
        (void)get_point_from_points_list(l, ref, &pt);
        uint32_t tmpsqd = sqd(&ptx, &pt);
        if (tmpsqd > lsqd) {
            lsqd = tmpsqd;
        }
        ptx = pt;
        (void)set_point_from_points_list(res, points_num, &pt);
        slot = cy->steps[slot].next;
        if (cy->steps[slot].ptref == ref) {
            slot = cy->steps[slot].next;
        }
    }
    return res;
}

struct points_list *short_cycle(const struct points_list *l) {
    struct complete_graph *cg = get_complete_graph_(l);
    if (cg == NULL) {
        return NULL;
    }
    struct cycles *cy = get_cycle_(cg);
    free(cg);
    if (cy == NULL) {
        return NULL;
    }
    struct points_list *result = link_points_(cy, l);
    free(cy);
    return result;
}
