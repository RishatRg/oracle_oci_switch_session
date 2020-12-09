#include <stdio.h>
#include <stdlib.h>
#include <oci.h>
#include <ocidfn.h>
#include <oratypes.h>

#include "checkerr.h"
#include "oci_common.h"

#define N 1010
#define M 32

FILE * prepare_file(const char * fileName)
{   
    FILE * file = NULL;
    if ((file = fopen(fileName, "w")) == NULL) {
        fprintf(stderr, "Could not open or create file (%s)! ", fileName);
        exit(EX_FAILURE);
    }
    fprintf(file, "{\n");
    fflush(file);
    return file;
}
void close_file(FILE *out)
{
    fprintf(out, "\n}");
    fclose(out);
}
void log_data_to_json_file(FILE *out, const char *itemName,  char data[N][M], int size)
{   
    fprintf(out, "\"%s\": [", itemName);
    for (int i = 0; i < size; i++) {
        if (i != size - 1)
            fprintf(out, "%s, ", data[i]);
        else
            fprintf(out, "%s", data[i]);
    }
    fprintf(out, "]");
    fflush(out);
}
void add_separator(FILE *out)
{
    fprintf(out, ",\n");
}

void select_with_switch_session(OCIEnv *envhp, OCIError *errhp, char *dbUser, char *userPassword, FILE *file)
{
	printf("----------START SELECT TEST----------\n");
	OCIServer *srvhp;
	OCISession *session1;
	OCISession *session2;
	OCIStmt *stmthp1;	
	OCIStmt *stmthp2;
	OCIDefine *dfnhp1 = (OCIDefine *)NULL;
	OCIDefine *dfnhp2 = (OCIDefine *)NULL;
	OCISvcCtx *svchp1;
	OCISvcCtx *svchp2;
    char buf1[N][M];
    char buf2[N][M];

    
	if(OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &svchp1,
				(ub4) OCI_HTYPE_SVCCTX,
				(size_t) 0, (dvoid **) 0) != OCI_SUCCESS ) {
		fprintf(stderr, "Fail to OCIHandleAlloc for service handle...\n");
		terminate_main(envhp, errhp);
		exit(EX_FAILURE);
	}
    if (OCIHandleAlloc((dvoid *)envhp, (dvoid **)&svchp2,
        (ub4)OCI_HTYPE_SVCCTX,
        (size_t)0, (dvoid **)0) != OCI_SUCCESS) {
        fprintf(stderr, "Fail to OCIHandleAlloc for service handle...\n");
        terminate_main(envhp, errhp);
        exit(EX_FAILURE);
    }

	/* Create connection */
    printf("- Create connection \n");
	initialize_server(envhp, errhp, &srvhp, "");

	/* Create first session */
    printf("- Create first session \n");
	initialize_user(envhp, errhp, srvhp, &session1, dbUser, userPassword, NULL, NULL);

    /* Create first select statement */
    printf("- Create first select statement \n");
    initialize_statement(envhp, errhp, &stmthp1, "SELECT ID FROM session_test_table");
    if (OCIDefineByPos(stmthp1, &dfnhp1, errhp, (ub4)1,
        (dvoid *)buf1, (sb4)32, SQLT_STR,
        (dvoid *)NULL, (ub2 *)NULL, (ub2 *)NULL,
        (ub4)OCI_DEFAULT)) {
        fprintf(stderr, "Fail to OCIHandleAlloc for statement handle");
        terminate_main(envhp, errhp);
        exit(EX_FAILURE);
    }

	/* Create second session */
    printf("- Create second session \n");
	initialize_user(envhp, errhp, srvhp, &session2, dbUser, userPassword, NULL, NULL);

	/* Create second select statement */
    printf("- Create second select statement \n");
	initialize_statement(envhp, errhp, &stmthp2, "SELECT ID FROM session_test_table");
	if(OCIDefineByPos(stmthp2, &dfnhp2, errhp, (ub4)1,
				(dvoid *)buf2, (sb4)32, SQLT_STR,
				(dvoid *)NULL, (ub2 *)NULL, (ub2 *)NULL,
				(ub4)OCI_DEFAULT))
	{
		fprintf(stderr, "Fail to checkerr(errhp,OCIHandleAlloc for statement handle");
		terminate_main(envhp, errhp);
		exit(EX_FAILURE);
	}
    printf("- Execute first statement \n");
    execute_statement(envhp, errhp, svchp1, session1, srvhp, stmthp1);

    printf("- Execute second statement \n");
	execute_statement(envhp, errhp, svchp2, session2, srvhp, stmthp2);


    printf("- Fetch second statement 100 rows \n");
	checkerr(errhp,OCIStmtFetch2(stmthp2, errhp, 100, OCI_DEFAULT, 0, OCI_DEFAULT));
    log_data_to_json_file(file, "SecondStmtResultBeforeSwitch_100", buf2, 100);
    add_separator(file);

	/* Switch to first session, fetch rest of rows */
    printf("- Switch to first session and fetch 1010 rows \n");
	checkerr(errhp,OCIStmtFetch2(stmthp1, errhp, 1010, OCI_DEFAULT, 0, OCI_DEFAULT));
    log_data_to_json_file(file,"FirstStmtResultAfterSwitch_1010", buf1, 1010);
    add_separator(file);

	/* Switch to second session, fetch few rows */
    printf("- Switch to second session and fetch 100 rows \n");
	checkerr(errhp,OCIStmtFetch2(stmthp2, errhp, 100, OCI_DEFAULT, 0, OCI_DEFAULT));
    log_data_to_json_file(file, "SecondStmtResultAfterSwitch_100", buf2, 100);
    add_separator(file);

	/* Close first session */
    printf("- Close first session \n");
	terminate_statement(errhp, stmthp1);
	terminate_user(envhp, errhp, srvhp, session1);

	/* Switch to second session, fetch rest of rows */
    printf("- Switch to second session, fetch rest of 810 rows  \n");
	checkerr(errhp,OCIStmtFetch2(stmthp2, errhp, 810, OCI_DEFAULT, 0, OCI_DEFAULT));
    log_data_to_json_file(file, "SecondStmtResultAfterSwitch_810", buf2, 810);
    close_file(file);

	/* Close second session */
    printf("- Close second session  \n");
	terminate_statement(errhp, stmthp2);
	terminate_user(envhp, errhp, srvhp, session2);

	/* Close connection */
    printf("- Close connection \n");
	terminate_server(errhp, srvhp);

	printf("----------END SELECT TEST----------\n");
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
         fprintf(stderr, "\nUsage: oci_test_switch_session <DBUSER> <DBUSER_PASSWORD> <FILE_PATH>\n\n");
         return 1;
     }
     
    char *dbuser = argv[1];
    char *userPassword = argv[2];
    char *resultFile = argv[3];
    FILE *out = prepare_file(resultFile);

	OCIEnv    *envhp;
	OCIError  *errhp;

	initialize_main(&envhp, &errhp);
	select_with_switch_session(envhp, errhp, dbuser, userPassword, out);
	terminate_main(envhp, errhp);
	return 0;
}
