#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// evaluation list;
typedef struct list {
    double val;
    char op;
    int nested;
    struct list *next;
} list;

// variable list;
typedef struct vlist {
    char *name;
    double val;
    bool isconst;
    struct vlist *next;
} vlist;

bool err_ocured = false;
double vlist_get(vlist *v, char *name);
void vlist_set(vlist *v, char *name, double val, bool isconst);
void vlist_print(vlist *v, FILE *fp);

double modulus(double n1, double n2) {
    while (n1 >= n2)
        n1 -= n2;
    return n1;
}

int op_val(char op) {
    switch (op) {
    case '+':
        return 1;
    case '-':
        return 1;
    case '*':
        return 2;
    case '/':
        return 2;
    case '%':
        return 2;
    case '^':
        return 3;
    default:
        return 0;
    }
}

bool do_op(list **l);

double str_list(char *str, vlist *v) {
    list *ret = malloc(sizeof(list));
    list *tmp = ret;
    int i = 0;
    int nested = 0;
    for (; str[i]; i++) {
        double left = 0, right = 0, pow10 = 10, val = 1;
        for (; str[i] == '-'; i++) {
            val *= -1;
            // for -(num1 op num2) calculation.
        }
        for (; str[i] == '(' || str[i] == ')' || (str[i] <= '9' && str[i] >= '0'); i++) {
            if (str[i] == '(')
                nested++;
            else if (str[i] == ')')
                nested--;
            else
                left = left * 10 + str[i] - '0';
        }
        if (str[i] == '.') {
            i++;
            for (; str[i] == '(' || str[i] == ')' || (str[i] <= '9' && str[i] >= '0'); i++) {
                if (str[i] == '(') {
                    nested++;
                } else if (str[i] == ')') {
                    nested--;
                } else {
                    right += (str[i] - '0') / pow10;
                    pow10 *= 10;
                }
            }
        }
        double higherval = 0;
        if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z') || str[i] == '_') {
            int size = 127;
            char *name_var = malloc(sizeof(char) * size + 1);
            for (int j = 0; str[i] != '\0' && j < size &&
                            ((str[i] <= 'z' && str[i] >= 'a') || (str[i] <= 'Z' && str[i] >= 'A') || str[i] == '_' ||
                             (str[i] >= '0' && str[i] <= '9'));
                 j++, i++) {
                name_var[j] = str[i];
            }
            for (; str[i] == '(' || str[i] == ')'; i++) {
                if (str[i] == '(') {
                    nested++;
                } else {
                    nested--;
                }
            }
            higherval = vlist_get(v, name_var); // It may exit the program.
            if (err_ocured) {
                return 0;
            }
        } else {
            higherval = (left + right);
        }
        tmp->next = malloc(sizeof(list));
        tmp = tmp->next;
        tmp->val = val * higherval;
        tmp->nested = nested;
        tmp->op = str[i];
    }
    tmp->next = NULL;
    tmp = ret->next;
    free(ret);
    list **ll = &tmp;
    while (do_op(ll))
        ;
    return (*ll)->val;
}

double op_single(double n1, char op, double n2) {
    switch (op) {
    case '*':
        return n1 * n2;
    case '/':
        return n1 / n2;
    case '+':
        return n1 + n2;
    case '-':
        return n1 - n2;
    case '^':
        return pow(n1, n2);
    case '%':
        return modulus(n1, n2);
    default:
        return 0;
    }
}

int list_nested_max(list *l) {
    int nested_max = 0;
    while (l != NULL) {
        if (l->nested > nested_max)
            nested_max = l->nested;
        l = l->next;
    }
    return nested_max;
}

int list_op_max(list *l, int nested) {
    int op_max = 0;
    while (l != NULL) {
        if (op_val(l->op) > op_max && l->nested == nested)
            op_max = op_val(l->op);
        l = l->next;
    }
    return op_max;
}

bool do_op(list **l) {
    list *tmp = *l;
    int nested = list_nested_max(tmp);
    int op_max = list_op_max(tmp, nested);

    if (tmp == NULL)
        return false;
    if (tmp->next == NULL)
        return false;
    else {
        if (tmp->nested == nested && op_val(tmp->op) == op_max) {
            tmp->next->val = op_single(tmp->val, tmp->op, tmp->next->val);
            tmp = tmp->next;
            free(*l);
            *l = tmp;
        } else {
            while (tmp->nested < nested)
                tmp = tmp->next;
            while (op_val(tmp->op) < op_max) {
                if (tmp->next->nested < nested)
                    break;
                tmp = tmp->next;
            }

            if (tmp->next == NULL) {
                tmp->nested--;
            } else {
                tmp->val = op_single(tmp->val, tmp->op, tmp->next->val);
                tmp->op = tmp->next->op;
                tmp->nested = tmp->next->nested;
                list *hold = tmp->next->next;
                free(tmp->next);
                tmp->next = hold;
            }
        }
        return true;
    }
}

bool syntax_check(const char *string) { return true; }

char *str_replace(char *str) {
    char *ret;
    int len_ret = 0;
    int nested = 0;
    int extra_opening_paran = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '-' && str[i + 1] == '(') {
            i++;
            len_ret += 4;
            nested++;
        } else if (str[i] != ' ' && str[i] != '\n' && str[i] != '\t') {
            if (str[i] == '(') {
                if (nested >= 0)
                    nested++;
            }
            if (str[i] == ')') {
                nested--;
            }
            if (nested < 0) {
                extra_opening_paran++;
                len_ret++;
                nested++;
            }
            len_ret++;
        }
    }

    ret = malloc(sizeof(char) * len_ret + 1);

    int i = 0, j = 0;
    for (; j < extra_opening_paran; j++) {
        ret[j] = '(';
    }
    for (; str[i] != '\0'; i++) {
        if (str[i] == '-' && str[i + 1] == '(') {
            i++;
            ret[j++] = '-';
            ret[j++] = '1';
            ret[j++] = '*';
            ret[j++] = '(';
        } else if (str[i] != ' ' && str[i] != '\n' && str[i] != '\t') {
            ret[j++] = str[i];
        }
    }

    ret[len_ret] = '\0';
    return ret;
}

double vlist_get(vlist *v, char *name) {
    while (v != NULL) {
        if (v->name != NULL && strcmp(v->name, name) == 0) {
            return v->val;
        }
        v = v->next;
    }
    fprintf(stderr, "Error: Variable \"%s\" haven't been declared.\n", name);
    err_ocured = true;
    return 0;
}

static inline int vlist_set_one(vlist *v, char *name, double val, bool isconst) {
    if (strcmp(v->name, name) == 0) {
        if (v->isconst) {
            fprintf(stderr, "Error: %s := %lf -> %lf is not possible, \"%s\" is constant.\n", name, v->val, val, name);
            return -1;
        } else if (isconst) {
            fprintf(stderr,
                    "Error %s = %lf => %s := %lf is not possible, \"%s\" have declared as non constant := operation is "
                    "assigning it to a constant.\n",
                    name, v->val, name, val, name);
            return -1;
        } else {
            v->val = val;
            return 1;
        }
    } else {
        return 0;
    }
}

void vlist_set(vlist *v, char *name, double val, bool isconst) {
    if (name == NULL)
        return;
    if (v == NULL)
        return;
    while (v->next != NULL) {
        if (vlist_set_one(v, name, val, isconst) == 1) {
            return;
        } else {
            v = v->next;
        }
    }
    if (vlist_set_one(v, name, val, isconst) == 0) {
        v->next = malloc(sizeof(vlist));
        v = v->next;
        v->name = name;
        v->val = val;
        v->isconst = isconst;
        v->next = NULL;
    }
}

void vlist_print(vlist *v, FILE *fp) {
    while (v != NULL) {
        fprintf(fp, "%s %s %lf\n", v->name, v->isconst ? ":=" : "=", v->val);
        v = v->next;
    }
}

int main(int argc, char **argv) {
    int size = 1023;
    char str[size];
    vlist *v = malloc(sizeof(vlist));
    v->isconst = false;
    v->name = malloc(sizeof(char) * 3 + 1);
    v->name[0] = 'a';
    v->name[1] = 'n';
    v->name[2] = 's';
    v->name[3] = '\0';
    v->val = 0;
    v->next = NULL;
    vlist_set(v, "true", 1, true);
    vlist_set(v, "false", 0, true);
    vlist_set(v, "PI", 3.1415926535897, true);
    vlist_set(v, "E", 2.7182818284590, true);
    while (true) {
        printf(" $ ");
        int i = 0;
        bool setvar = false;
        bool syntax_err = false;
        for (; i < size; i++) {
            scanf("%c", &str[i]);
            if (setvar && (str[i] == '=' || str[i] == ':')) {
                fprintf(stderr, "Error: syntax, usage: <var>\n                      <var> = <expression>\n             "
                                "         <const var>\n                      <const var> := <expression>\n");
                syntax_err = true;
            } else if (str[i] == '=' || str[i] == ':') {
                setvar = true;
            }
            if (str[i] == '\n') {
                break;
            }
        }
        str[i + 1] = '\0';
        char *ret = str_replace(str);
        if (setvar) {
            int i = 0;
            bool isconst = false;
            for (i = 0; ret[i] != '\0'; i++) {
                if (ret[i] == ':') {
                    isconst = true;
                    i++;
                    if (ret[i] != '\0') {
                        if (ret[i] == '=') {
                            break;
                        }
                    }
                }
                if (ret[i] == '=') {
                    break;
                }
            }
            char *var_name = malloc(sizeof(char) * i + 1);
            i = 0;
            if (!((ret[i] <= 'Z' && ret[i] >= 'A') || (ret[i] >= 'a' && ret[i] <= 'z') || ret[i] == '_')) {
                syntax_err = true;
                fprintf(stderr, "Error: Variable name can only start with A to Z, a to z and _\n");
            }
            for (i = 0; ret[i] != '\0'; i++) {
                if (ret[i] == ':') {
                    isconst = true;
                    i++;
                    if (ret[i] != '\0' && ret[i] == '=') {
                        break;
                    } else {
                        fprintf(stderr,
                                "Error: syntax, usage: <var>\n                      <var> = <expression>\n             "
                                "         <const var>\n                      <const var> := <expression>\n");
                        syntax_err = true;
                        break;
                    }
                } else if (ret[i] == '=') {
                    break;
                }
                if ((ret[i] <= 'Z' && ret[i] >= 'A') || (ret[i] >= 'a' && ret[i] <= 'z') || ret[i] == '_' ||
                    (ret[i] >= '0' && ret[i] <= '9')) {
                    var_name[i] = ret[i];
                } else {
                    syntax_err = true;
                    if (ret[i] != '\0' && ret[i + 1] == '=') {
                        fprintf(stderr,
                                "Error: syntax, usage: <var>\n                      <var> = <expression>\n             "
                                "         <const var>\n                      <const var> := <expression>\n");
                    } else {
                        fprintf(stderr, "Error: Variable name can continue with A to Z, a to z and _\n");
                    }
                }
            }
            var_name[i++] = '\0';
            if (!syntax_err) {
                for (int j = i; ret[j] != '\0'; j++) {
                    if (!((ret[i] <= 'Z' && ret[i] >= 'A') || (ret[i] >= 'a' && ret[i] <= 'z') || ret[i] == '_' ||
                          (ret[i] >= '0' && ret[i] <= '9'))) {
                        syntax_err = true;
                        fprintf(stderr, "Error: Variable name can only continue with A to Z, a to z, 0 to 9 and _\n");
                        break;
                    }
                }
                if (!syntax_err) {
                    if (strcmp(var_name, "quit") == 0 || strcmp(var_name, "exit") == 0 ||
                        strcmp(var_name, "dump") == 0) {
                        fprintf(stderr, "%s is a function!, you cannot assign it.\n", var_name);
                    } else if (strcmp(var_name, "save") == 0) {
                        for (int j = 0; (ret + i)[j] != '\0'; j++) {
                            if ((ret + i)[j] == '\n') {
                                (ret + i)[j] = '\0';
                                break;
                            }
                        }
                        FILE *fp = fopen(ret + i, "w");
                        vlist_print(v, fp);
                        fclose(fp);
                    } else {
                        double val = str_list(ret + i, v);
                        if (err_ocured) {
                            err_ocured = false;
                        } else {
                            vlist_set(v, var_name, val, isconst);
                        }
                    }
                }
            }
        } else {
            if (strcmp(ret, "quit") == 0 || strcmp(ret, "exit") == 0) {
                exit(1);
            } else if (strcmp(ret, "dump") == 0 || strcmp(ret, "save") == 0) {
                vlist_print(v, stderr);
            } else {
                double val = str_list(ret, v);
                if (err_ocured) {
                    err_ocured = false;
                } else {
                    vlist_set(v, "ans", val, false);
                    fprintf(stderr, "ans = %lf\n", val);
                }
            }
        }
        free(ret);
    }
    return 0;
}
