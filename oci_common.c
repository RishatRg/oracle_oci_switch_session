#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oci_common.h"

void error_report(OCIError *errhp, CONST text *op)
{
	text  msgbuf[2000];
	sb4   errcode = 0;

	fprintf(stderr,"ORACLE error during %s\n", op);
	OCIErrorGet ((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode,
			msgbuf, (ub4) sizeof(msgbuf), (ub4) OCI_HTYPE_ERROR);
	fprintf(stderr,"ERROR CODE = %d\n", errcode);
	fprintf(stderr,"%s\n", msgbuf);
	exit(EX_FAILURE);
}

void initialize_main(OCIEnv **envhpp, OCIError **errhpp)
{
	if(OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0, (dvoid * (*)()) 0,
				(dvoid * (*)())0, (void (*)()) 0 ) != OCI_SUCCESS) {
		fprintf(stdout, "Fail to checkerr(errhp,OCIInitialize...\n");
		exit(EX_FAILURE);
	}

	if(OCIEnvInit( (OCIEnv **) envhpp, (ub4) OCI_DEFAULT,
				(size_t) 0, (dvoid **) 0 ) != OCI_SUCCESS) {
		fprintf(stdout, "Fail to checkerr(errhp,OCIEnvInit for service handle...\n");
		exit(EX_FAILURE);
	}

	/* Get Error Handle */
	if(OCIHandleAlloc( (dvoid *) *envhpp, (dvoid **) errhpp,
				(ub4) OCI_HTYPE_ERROR,
				(size_t) 0, (dvoid **) 0) != OCI_SUCCESS) {
		fprintf(stdout, "Fail to checkerr(errhp,OCIHandleAlloc for error handle...\n");
		exit(EX_FAILURE);
	}
	return;
}

void terminate_main(OCIEnv *envhp, OCIError *errhp)
{
	checkerr(errhp,OCIHandleFree((dvoid *) errhp, (ub4) OCI_HTYPE_ERROR));
	checkerr(errhp,OCIHandleFree((dvoid *) envhp, (ub4) OCI_HTYPE_ENV));
}

void initialize_server(OCIEnv *envhp, OCIError *errhp, OCIServer **srvhpp, char *dblink)
{
	if(OCIHandleAlloc( (dvoid *) envhp, (dvoid **) srvhpp,
				(ub4) OCI_HTYPE_SERVER,
				(size_t) 0, (dvoid **) 0) != OCI_SUCCESS) {
		fprintf(stdout, "Fail to checkerr(errhp,OCIHandleAlloc for server handle...\n");
		exit(EX_FAILURE);
	}

	/* Initialize Server Handle */
	checkerr(errhp,OCIServerAttach(*srvhpp, errhp, (text *)dblink, strlen(dblink), 0));
}

void terminate_server(OCIError *errhp, OCIServer *srvhp)
{
	checkerr(errhp,OCIServerDetach(srvhp, errhp, OCI_DEFAULT));
	checkerr(errhp,OCIHandleFree((dvoid *) srvhp, (ub4) OCI_HTYPE_SERVER));
}

/*
 * If proxy is not NULL, create proxied session
 * If primary is not NULL, create migratable session
 */
void initialize_user(OCIEnv *envhp, OCIError *errhp, OCIServer *srvhp, OCISession **userhpp, char *name, char *password, char *proxy, OCISession *primary)
{
	OCISvcCtx *svchp;

	if (proxy == NULL) {
		fprintf (stdout, "Authentication for %s is in progress...\n", name);
	} else {
		fprintf (stdout, "Authentication for %s is in progress...\n", proxy);
	}

	/* Temporary Service Context */
	if(OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &svchp,
				(ub4) OCI_HTYPE_SVCCTX,
				(size_t) 0, (dvoid **) 0) != OCI_SUCCESS ) {
		fprintf(stdout, "Fail to checkerr(errhp,OCIHandleAlloc for service handle...\n");
		terminate_main(envhp, errhp);
		exit(EX_FAILURE);
	}
	if(OCIHandleAlloc( (dvoid *) envhp, (dvoid **) userhpp,
				(ub4) OCI_HTYPE_SESSION,
				(size_t) 0, (dvoid **) 0) != OCI_SUCCESS ) {
		fprintf(stdout, "Fail to checkerr(errhp,OCIHandleAlloc for user handle...\n");
		terminate_main(envhp, errhp);
		exit(EX_FAILURE);
	}

	if (primary != NULL) {
		/* Set the Primary Session  handle in the service handle */
		checkerr(errhp,OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, primary, 0,
					OCI_ATTR_SESSION, errhp));
	}

	/* Set the server handle in service handle */
	checkerr(errhp,OCIAttrSet (svchp, OCI_HTYPE_SVCCTX, srvhp, 0,
				OCI_ATTR_SERVER, errhp));

	/* set the username/password in user handle */
	checkerr(errhp,OCIAttrSet(*userhpp, OCI_HTYPE_SESSION, name, strlen(name),
				OCI_ATTR_USERNAME, errhp));

	checkerr(errhp,OCIAttrSet(*userhpp, OCI_HTYPE_SESSION, password, strlen(password),
				OCI_ATTR_PASSWORD, errhp));

	if (proxy != NULL) {
		checkerr(errhp,OCIAttrSet(*userhpp, OCI_HTYPE_SESSION, proxy, strlen(proxy),
					OCI_ATTR_PROXY_CLIENT, errhp));
	}

	/* Authenticate */
	if (primary != NULL) {
		checkerr(errhp,OCISessionBegin (svchp, errhp, *userhpp,
					OCI_CRED_RDBMS, OCI_MIGRATE));
	} else {
		checkerr(errhp,OCISessionBegin (svchp, errhp, *userhpp,
					OCI_CRED_RDBMS, OCI_DEFAULT));
	}

	/* Free Temporary Service Context */
	checkerr(errhp,OCIHandleFree((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX));

	if (proxy == NULL) {
		fprintf (stdout, "Authentication for %s successful.\n", name);
	} else {
		fprintf (stdout, "Authentication for %s successful.\n", proxy);
	}
}

void terminate_user(OCIEnv *envhp, OCIError *errhp, OCIServer *srvhp, OCISession *userhp)
{
	OCISvcCtx *svchp;
	/* Temporary Service Context */
	if(OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &svchp,
				(ub4) OCI_HTYPE_SVCCTX,
				(size_t) 0, (dvoid **) 0) != OCI_SUCCESS ) {
		fprintf(stdout, "Fail to OCIHandleAlloc for service handle...\n");
		terminate_main(envhp, errhp);
		exit(EX_FAILURE);
	}

	fprintf(stdout, "Logging off...\n");

	checkerr(errhp,OCIAttrSet (svchp, OCI_HTYPE_SVCCTX, srvhp, 0,
				OCI_ATTR_SERVER, errhp));

	/* Set the Authentication handle in the service handle */
	checkerr(errhp,OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, userhp, 0,
				OCI_ATTR_SESSION, errhp));

	checkerr(errhp,OCISessionEnd(svchp, errhp, userhp, (ub4) 0));

	/* Free Temporary Service Context */
	checkerr(errhp,OCIHandleFree((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX));
	fprintf(stdout, "Logged off.\n");
}

void initialize_statement(OCIEnv *envhp, OCIError *errhp, OCIStmt **stmhpp, char *stmt)
{
	/* Get statement handles */
	if(OCIHandleAlloc( (dvoid *)envhp, (dvoid **) stmhpp,
				(ub4) OCI_HTYPE_STMT,
				(size_t) 0, (dvoid **) 0) != OCI_SUCCESS ) {
		fprintf(stdout, "Fail to OCIHandleAlloc for statement handle\n");
		terminate_main(envhp, errhp);
		exit(EX_FAILURE);
	}

	if(OCIStmtPrepare(*stmhpp, errhp, (text *)stmt,
				(ub4)strlen(stmt),
				(ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT)) {
		fprintf(stdout, "Fail to OCIHandleAlloc for statement handle\n");
		terminate_main(envhp, errhp);
		exit(EX_FAILURE);
	}

	/* /1* OCIDefineByPos *1/ */
	/* checkerr(errhp,OCIDefineByPos(*stmhpp, &dfnhp, errhp, (ub4)1, */
	/* 			(dvoid *)username, (sb4)sizeof(username), SQLT_CHR, */
	/* 			(   dvoid *)&userind, &userlen, (ub2 *)NULL, */
	/* 			(ub4)OCI_DEFAULT)) */
	/* { */
	/* 	fprintf(stdout, "Fail to checkerr(errhp,OCIHandleAlloc for statement handle\n"); */
	/* 	terminate_main(envhp, errhp); */
	/* 	exit(EX_FAILURE); */
	/* } */
}

void execute_statement(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp, OCISession *userhp, OCIServer *srvhp, OCIStmt *stmhp)
{
	sword retval;
	int free = 0;

	if (svchp == NULL) {
		free = 1;
		/* Temporary Service Context */
		if(OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &svchp,
					(ub4) OCI_HTYPE_SVCCTX,
					(size_t) 0, (dvoid **) 0) != OCI_SUCCESS ) {
			fprintf(stdout, "Fail to OCIHandleAlloc for service handle...");
			terminate_main(envhp, errhp);
			exit(EX_FAILURE);
		}
	}

	if(OCIAttrSet (svchp, OCI_HTYPE_SVCCTX, srvhp, 0,
				OCI_ATTR_SERVER, errhp) != OCI_SUCCESS);

	/* Set the Authentication handle in the service handle */
	if(OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, userhp, 0,
				OCI_ATTR_SESSION, errhp) != OCI_SUCCESS);

	retval = OCIStmtExecute(svchp, stmhp, errhp, (ub4)0, (ub4)0,
			(OCISnapshot*)0, (OCISnapshot*)0,
			(ub4)OCI_DEFAULT);

	if (retval != OCI_SUCCESS) {
		error_report(errhp, (text *)"OCIStmtExecute");
		exit(-1);
	}

	if (free == 1) {
		/* Free Temporary Service Context */
		checkerr(errhp,OCIHandleFree((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX));
	}
}

void terminate_statement(OCIError *errhp, OCIStmt *stmhp)
{
  checkerr(errhp,OCIHandleFree((dvoid *) stmhp, (ub4) OCI_HTYPE_STMT));
}
