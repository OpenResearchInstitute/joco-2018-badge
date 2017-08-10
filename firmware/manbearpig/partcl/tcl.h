 /*****************************************************************************
 * (C) Copyright 2017 AND!XOR LLC (http://andnxor.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL AUGUST 1ST, 2017 then,
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@andrewnriley
 * 	@lacosteaef
 * 	@bitstr3m
 *****************************************************************************/
#ifndef PARTCL_TCL_H_
#define PARTCL_TCL_H_

typedef char tcl_value_t;

struct tcl {
	struct tcl_env *env;
	struct tcl_cmd *cmds;
	tcl_value_t *result;
};

typedef int (*tcl_cmd_fn_t)(struct tcl *, tcl_value_t *, void *);

/* Token type and control flow constants */
enum {
	TCMD, TWORD, TPART, TERROR
};
enum {
	FERROR, FNORMAL, FRETURN, FBREAK, FAGAIN
};

extern tcl_value_t *tcl_alloc(const char *s, size_t len);
extern void tcl_destroy(struct tcl *tcl);
extern tcl_value_t *tcl_dup(tcl_value_t *v);
extern int tcl_eval(struct tcl *tcl, const char *s, size_t len);
extern void tcl_free(tcl_value_t *v);
extern void tcl_init(struct tcl *tcl);
extern int tcl_int(tcl_value_t *v);
extern int tcl_length(tcl_value_t *v);
extern tcl_value_t *tcl_list_at(tcl_value_t *v, int index);
extern void tcl_register(struct tcl *tcl, const char *name, tcl_cmd_fn_t fn,
		int arity, void *arg);
extern int tcl_result(struct tcl *tcl, int flow, tcl_value_t *result);
extern const char *tcl_string(tcl_value_t *v);
extern tcl_value_t *tcl_var(struct tcl *tcl, tcl_value_t *name, tcl_value_t *v);

#endif /* PARTCL_TCL_H_ */
