#pragma once
#include <oci.h>
#include <ocidfn.h>

#include "checkerr.h"

#define EX_FAILURE -1

void error_report(OCIError *errhp, CONST text *op);

void initialize_main(OCIEnv **envhpp, OCIError **errhpp);

void terminate_main(OCIEnv *envhp, OCIError *errhp);

void initialize_server(OCIEnv *envhp, OCIError *errhp, OCIServer **srvhpp, char *dblink);

void terminate_server(OCIError *errhp, OCIServer *srvhp);

void initialize_user(OCIEnv *envhp, OCIError *errhp, OCIServer *srvhp, OCISession **userhpp, char *name, char *password, char *proxy, OCISession *primary);

void terminate_user(OCIEnv *envhp, OCIError *errhp, OCIServer *srvhp, OCISession *userhp);

void initialize_statement(OCIEnv *envhp, OCIError *errhp, OCIStmt **stmhpp, char *stmt);

void execute_statement(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp, OCISession *userhp, OCIServer *srvhp, OCIStmt *stmhp);

void terminate_statement(OCIError *errhp, OCIStmt *stmhp);
