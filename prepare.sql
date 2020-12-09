set echo on;

drop user ocitest cascade;
create user ocitest identified by ocitest;
grant connect, resource , UNLIMITED TABLESPACE to ocitest;

connect ocitest/ocitest;

create table session_test_table (id NUMBER(5) NOT NULL,
                                name VARCHAR2(15) NOT NULL,
                                email VARCHAR2(15) NOT NULL,
                                phone NUMBER(10) NOT NULL,
                                address VARCHAR2(15) NOT NULL,
                                country VARCHAR2(15) NOT NULL,
                                city VARCHAR2(15) NOT NULL,
                                job VARCHAR2(500) NOT NULL,
                                company VARCHAR2(500) NOT NULL,
                                salary NUMBER(10) NOT NULL);

DECLARE
    i NUMBER;
BEGIN
    FOR i IN 1 .. 1010 LOOP
        INSERT INTO session_test_table (id, name, email, phone, address, country, city, job, company, salary)
        VALUES (i, dbms_random.string('A', 15), dbms_random.string('A', 15), dbms_random.value(1000000000, 9000000000),
            dbms_random.string('A', 15), dbms_random.string('A', 15), dbms_random.string('A', 15), dbms_random.string('A', 500),
            dbms_random.string('A', 500), dbms_random.value(1000000000, 9000000000));
    END LOOP;
    COMMIT;
END;
/
