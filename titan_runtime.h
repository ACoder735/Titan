#ifndef TITAN_RUNTIME_H
#define TITAN_RUNTIME_H

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct TitanVar TitanVar;

typedef struct {
    TitanVar* data;
    int count;
    int capacity;
} TitanArray;

struct TitanVar {
    int type; double num; char* str; TitanArray* arr;
};

#define TITAN_NULL ((TitanVar){0, 0, 0, 0})
#define titan_num(n) ((TitanVar){1, n, 0, 0})
#define titan_str(s) ((TitanVar){2, 0, s, 0})

int titan_bool(TitanVar v) { if (v.type == 0) return 0; if (v.type == 1) return v.num != 0; if (v.type == 2) return v.str != NULL && strlen(v.str) > 0; return 0; }

TitanVar titan_new_array() {
    TitanArray* a = malloc(sizeof(TitanArray));
    a->data = malloc(sizeof(TitanVar) * 4);
    a->count = 0; a->capacity = 4;
    return (TitanVar){3, 0, 0, a};
}

void _titan_ensure_space(TitanArray* a, int n) {
    if (a->count + n >= a->capacity) {
        while (a->count + n >= a->capacity) a->capacity *= 2;
        a->data = realloc(a->data, sizeof(TitanVar) * a->capacity);
    }
}

TitanVar titan_arr_push_result(TitanVar list, TitanVar item) {
    if(list.type != 3) return TITAN_NULL;
    _titan_ensure_space(list.arr, 1);
    list.arr->data[list.arr->count++] = item;
    return list;
}

void titan_arr_push(TitanVar list, TitanVar item) {
    if(list.type != 3) return;
    _titan_ensure_space(list.arr, 1);
    list.arr->data[list.arr->count++] = item;
}

TitanVar titan_arr_get(TitanVar list, int index) {
    if(list.type != 3 || index < 0 || index >= list.arr->count) return TITAN_NULL;
    return list.arr->data[index];
}

void titan_arr_replace(TitanVar list, int index, TitanVar val) {
    if(list.type != 3 || index < 0 || index >= list.arr->count) return;
    list.arr->data[index] = val;
}

void titan_arr_insert(TitanVar list, int index, TitanVar item) {
    if(list.type != 3 || index < 0 || index > list.arr->count) return;
    _titan_ensure_space(list.arr, 1);
    for (int i = list.arr->count; i > index; i--) list.arr->data[i] = list.arr->data[i-1];
    list.arr->data[index] = item;
    list.arr->count++;
}

void titan_arr_remove(TitanVar list, int index) {
    if(list.type != 3 || index < 0 || index >= list.arr->count) return;
    for (int i = index; i < list.arr->count - 1; i++) list.arr->data[i] = list.arr->data[i+1];
    list.arr->count--;
}

TitanVar titan_arr_concat(TitanVar a, TitanVar b) {
    if(a.type != 3 || b.type != 3) return TITAN_NULL;
    TitanVar result = titan_new_array();
    _titan_ensure_space(result.arr, a.arr->count + b.arr->count);
    for(int i=0; i<a.arr->count; i++) result.arr->data[result.arr->count++] = a.arr->data[i];
    for(int i=0; i<b.arr->count; i++) result.arr->data[result.arr->count++] = b.arr->data[i];
    return result;
}

TitanVar titan_len(TitanVar v) {
    if (v.type == 3) return titan_num(v.arr->count);
    if (v.type == 2) return titan_num(strlen(v.str));
    return titan_num(0);
}

char* _titan_val_to_str(TitanVar v) {
    char* buf = malloc(256); buf[0] = 0;
    if (v.type == 1) { sprintf(buf, "%g", v.num); }
    else if (v.type == 2) { strcpy(buf, v.str); }
    else if (v.type == 0) { strcpy(buf, "NULL"); }
    else if (v.type == 3) {
        strcpy(buf, "[");
        for(int i=0; i<v.arr->count; i++) {
            if(i>0) strcat(buf, ", ");
            char* inner = _titan_val_to_str(v.arr->data[i]);
            strcat(buf, inner);
            free(inner);
        }
        strcat(buf, "]");
    }
    return buf;
}

TitanVar titan_tostring(TitanVar v) {
    return titan_str(_titan_val_to_str(v));
}

TitanVar titan_tonum(TitanVar v) { if (v.type == 1) return v; if (v.type == 2) return titan_num(atof(v.str)); return titan_num(0); }

void titan_print(TitanVar v) {
    if (v.type == 2) printf("%s", v.str);
    else { printf("RUNTIME ERROR: Cannot print non-String types. Use Str(val).\n"); exit(1); }
}
void titan_println(TitanVar v) { titan_print(v); printf("\n"); }

TitanVar titan_input(TitanVar prompt) { titan_print(prompt); fflush(stdout); char* buffer = malloc(256); if (fgets(buffer, 256, stdin) != NULL) { buffer[strcspn(buffer, "\n")] = 0; buffer[strcspn(buffer, "\r")] = 0; return titan_str(buffer); } return TITAN_NULL; }

TitanVar titan_add(TitanVar a, TitanVar b) {
    if (a.type == 1 && b.type == 1) return titan_num(a.num + b.num);
    if (a.type == 2 && b.type == 2) { int len = strlen(a.str) + strlen(b.str) + 1; char* res = malloc(len); strcpy(res, a.str); strcat(res, b.str); return titan_str(res); }
    if (a.type == 3 && b.type == 3) return titan_arr_concat(a, b);
    printf("RUNTIME ERROR: Cannot add mismatched types.\n"); exit(1);
}

TitanVar titan_sub(TitanVar a, TitanVar b) { if (a.type != 1 || b.type != 1) { printf("RUNTIME ERROR: '-' requires Numbers.\n"); exit(1); } return titan_num(a.num - b.num); }
TitanVar titan_mul(TitanVar a, TitanVar b) { if (a.type != 1 || b.type != 1) { printf("RUNTIME ERROR: '*' requires Numbers.\n"); exit(1); } return titan_num(a.num * b.num); }
TitanVar titan_div(TitanVar a, TitanVar b) { if (a.type != 1 || b.type != 1) { printf("RUNTIME ERROR: '/' requires Numbers.\n"); exit(1); } return titan_num(a.num / b.num); }
TitanVar titan_mod(TitanVar a, TitanVar b) { if (a.type != 1 || b.type != 1) { printf("RUNTIME ERROR: '%%' requires Numbers.\n"); exit(1); } return titan_num(fmod(a.num, b.num)); }

TitanVar titan_neg(TitanVar a) { if(a.type != 1) { printf("RUNTIME ERROR: '-' requires Number.\n"); exit(1); } return titan_num(-a.num); }
TitanVar titan_not(TitanVar a) { return titan_num(!titan_bool(a)); }
TitanVar titan_and(TitanVar a, TitanVar b) { return titan_num(titan_bool(a) && titan_bool(b)); }
TitanVar titan_or(TitanVar a, TitanVar b) { return titan_num(titan_bool(a) || titan_bool(b)); }

TitanVar titan_eq(TitanVar a, TitanVar b) { if (a.type != b.type) return titan_num(0); if (a.type == 0) return titan_num(1); if (a.type == 2) return titan_num(strcmp(a.str, b.str) == 0); return titan_num(a.num == b.num); }
TitanVar titan_neq(TitanVar a, TitanVar b) { return titan_num(!titan_bool(titan_eq(a, b))); }
TitanVar titan_lt(TitanVar a, TitanVar b) { return titan_num(a.num < b.num); }
TitanVar titan_gt(TitanVar a, TitanVar b) { return titan_num(a.num > b.num); }
TitanVar titan_lte(TitanVar a, TitanVar b) { return titan_num(a.num <= b.num); }
TitanVar titan_gte(TitanVar a, TitanVar b) { return titan_num(a.num >= b.num); }

void titan_clear() { system("cls"); }
void titan_settitle(TitanVar t) { if(t.type != 2) return; char cmd[256]; sprintf(cmd, "title %s", t.str); system(cmd); }

void titan_color(TitanVar c) {
    if (c.type == 1) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (int)c.num);
}

#endif
