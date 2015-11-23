--
-- Not used by the plugin.
-- Just here so the tables show up in the tree.
--

CREATE TABLE Tags(
	Idx INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	Tag TEXT NOT NULL,
	File TEXT NOT NULL,
	Line INTEGER,
	Pattern TEXT,
	Type TEXT,
	Language TEXT,
	MemberOf TEXT,
	MemberOfType INTEGER,
	Inherits TEXT,
	Signature TEXT,
	Access TEXT,
	Implementation TEXT,
	ThisFileOnly INTEGER,
	Unrecognized TEXT
);

CREATE INDEX TagsName ON Tags(Tag);
CREATE INDEX TagsLangType ON Tags(Language, Type);
CREATE INDEX TagsType ON Tags(Type);
CREATE INDEX TagsLangMember ON Tags(Language, MemberOf);

-- Not used yet
CREATE TABLE Settings(
	Key TEXT PRIMARY KEY,
	Value TEXT
);
