create table sda_saves
(
    save_id         INTEGER primary key autoincrement,
    date            INTEGER,
    insertsCount    INTEGER,
    updatesCount    INTEGER,
    deletesCount    INTEGER
);

create table sda_ghidra_sync
(
    sync_id         INTEGER primary key autoincrement,
    date            INTEGER,
    type            INTEGER,
    comment         TEXT,
    objectsCount    INTEGER
);

CREATE TABLE "sda_address_spaces"
(
	"as_id"	        INTEGER,
	"name"	        TEXT,
	"comment"	    TEXT,
    "save_id"	    INTEGER,
    "deleted"	    INTEGER DEFAULT 0,
	PRIMARY KEY("as_id")
);

CREATE TABLE "sda_images"
(
	"image_id"	            INTEGER,
    "parent_image_id"       INTEGER,
    "type"	                INTEGER,
	"name"	                TEXT,
	"comment"	            TEXT,
    "addr_space_id"         INTEGER,
    "global_table_id"       INTEGER,
    "func_body_table_id"    INTEGER,
    "json_instr_pool"       TEXT,
    "json_vfunc_calls"      TEXT,
    "json_func_graphs"      TEXT,
    "save_id"	            INTEGER,
    "deleted"	            INTEGER DEFAULT 0,
	PRIMARY KEY("image_id")
);

CREATE TABLE "sda_symbols"
(
	"symbol_id"	            INTEGER PRIMARY KEY AUTOINCREMENT,
    "type"                  INTEGER,
	"name"	                TEXT,
	"type_id"	            INTEGER NOT NULL,
	"pointer_lvl"	        TEXT,
	"comment"	            TEXT,
    "json_extra"            TEXT,
	"save_id"	            INTEGER,
	"ghidra_sync_id"	    INTEGER,
	"deleted"	            INTEGER DEFAULT 0
);

CREATE TABLE "sda_symbol_tables"
(
	"sym_table_id"	INTEGER,
	"type"	        INTEGER,
    "json_symbols"  TEXT,
    "save_id"	    INTEGER,
    "deleted"	    INTEGER DEFAULT 0,
	PRIMARY KEY("sym_table_id")
);

create table sda_functions
(
    func_id                 INTEGER primary key autoincrement,
    func_symbol_id          INTEGER,
    stack_sym_table_id      INTEGER DEFAULT 0,
    image_id                INTEGER,
    save_id                 INTEGER,
    ghidra_sync_id          INTEGER,
    deleted                 INTEGER DEFAULT 0
);

create table sda_triggers
(
    trigger_id      INTEGER primary key autoincrement,
    type            INTEGER,
    name            TEXT,
    comment         TEXT,
    json_extra      TEXT,
    save_id         INTEGER,
    deleted         INTEGER DEFAULT 0
);

create table sda_types
(
    type_id             INTEGER  primary key autoincrement,
    "group"             INTEGER,
    name                TEXT unique,
    comment             TEXT,
    json_extra          TEXT,
    save_id             INTEGER,
    ghidra_sync_id      INTEGER,
    deleted             INTEGER DEFAULT 0
);
INSERT INTO sda_types (name) VALUES ('reserved');
UPDATE SQLITE_SEQUENCE SET seq=1000 WHERE name='sda_types';