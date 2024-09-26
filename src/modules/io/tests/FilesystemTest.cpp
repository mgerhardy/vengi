/**
 * @file
 */

#include "app/tests/AbstractTest.h"
#include "core/tests/TestHelper.h"
#include "io/Filesystem.h"
#include "core/Algorithm.h"
#include "core/Enum.h"
#include "core/StringUtil.h"
#include "io/FormatDescription.h"
#include <gtest/gtest.h>

namespace io {

class FilesystemTest : public app::AbstractTest {};

::std::ostream &operator<<(::std::ostream &ostream, const core::DynamicArray<io::FilesystemEntry> &val) {
	for (const auto &e : val) {
		ostream << e.name.c_str() << " - " << core::enumVal(e.type) << ", ";
	}
	return ostream;
}

TEST_F(FilesystemTest, testListDirectory) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysCreateDir(core::Path("listdirtest/dir1")));
	EXPECT_TRUE(fs.sysWrite("listdirtest/dir1/ignored", "ignore"));
	EXPECT_TRUE(fs.sysWrite("listdirtest/dir1/ignoredtoo", "ignore"));
	EXPECT_TRUE(fs.sysWrite("listdirtest/file1", "1"));
	EXPECT_TRUE(fs.sysWrite("listdirtest/file2", "2"));
	core::DynamicArray<io::FilesystemEntry> entities;
	fs.list("listdirtest/", entities, "");
	EXPECT_FALSE(entities.empty());
	EXPECT_EQ(3u, entities.size()) << entities;
	entities.sort([](const io::FilesystemEntry &first, const io::FilesystemEntry &second) {
		return first.name > second.name;
	});
	if (entities.size() >= 3) {
		EXPECT_EQ("dir1", entities[0].name) << entities[0].name.c_str();
		EXPECT_EQ("file1", entities[1].name) << entities[1].name.c_str();
		EXPECT_EQ("file2", entities[2].name) << entities[2].name.c_str();
		EXPECT_EQ(io::FilesystemEntry::Type::dir, entities[0].type) << entities[0].name.c_str();
		EXPECT_EQ(io::FilesystemEntry::Type::file, entities[1].type) << entities[1].name.c_str();
		EXPECT_EQ(io::FilesystemEntry::Type::file, entities[2].type) << entities[2].name.c_str();
	}
	fs.shutdown();
}

TEST_F(FilesystemTest, testDirectoryExists) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysCreateDir(core::Path("testdirexists")));
	EXPECT_TRUE(fs.sysIsReadableDir("testdirexists"));
	EXPECT_TRUE(fs.exists("testdirexists"));
	EXPECT_FALSE(fs.sysIsReadableDir("testdirdoesnotexist"));
	EXPECT_FALSE(fs.exists("testdirdoesnotexist"));
}

TEST_F(FilesystemTest, testFileExists) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.exists("iotest.txt"));
	EXPECT_FALSE(fs.exists("iotestdoesnotexist.txt"));
}

TEST_F(FilesystemTest, testListDirectoryFilter) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysCreateDir(core::Path("listdirtestfilter")));
	EXPECT_TRUE(fs.sysWrite("listdirtestfilter/image.Png", "1"));
	EXPECT_TRUE(fs.sysWrite("listdirtestfilter/foobar.foo", "1"));
	EXPECT_TRUE(fs.sysWrite("listdirtestfilter/foobar.png", "1"));
	EXPECT_TRUE(fs.sysWrite("listdirtestfilter/foobar.jpeg", "1"));
	EXPECT_TRUE(fs.sysWrite("listdirtestfilter/foobar.jpg", "1"));
	core::DynamicArray<io::FilesystemEntry> entities;
	const FormatDescription desc = {"", {"jpeg", "jpg"}, {}, 0u};
	const core::String &jpegFilePattern = convertToFilePattern(desc);
	fs.list("listdirtestfilter/", entities, jpegFilePattern);
	EXPECT_FALSE(entities.empty());
	EXPECT_EQ(2u, entities.size()) << entities;
	entities.sort([](const io::FilesystemEntry &first, const io::FilesystemEntry &second) {
		return first.name > second.name;
	});
	if (entities.size() >= 2) {
		EXPECT_EQ(io::FilesystemEntry::Type::file, entities[0].type);
		EXPECT_EQ("foobar.jpeg", entities[0].name);
		EXPECT_EQ(io::FilesystemEntry::Type::file, entities[1].type);
		EXPECT_EQ("foobar.jpg", entities[1].name);
	}
	fs.shutdown();
}

TEST_F(FilesystemTest, testAbsolutePath) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysCreateDir(core::Path("absolutePathInCurDir")));
	const core::String &absolutePathInCurDir = fs.sysAbsolutePath("absolutePathInCurDir");
	EXPECT_EQ(core::string::path(fs.sysCurrentDir().str(), "absolutePathInCurDir"), absolutePathInCurDir);
	EXPECT_TRUE(core::string::isAbsolutePath(absolutePathInCurDir));
	const core::String &abspath = fs.sysAbsolutePath("");
	EXPECT_EQ(fs.sysCurrentDir(), abspath);
	fs.shutdown();
}

TEST_F(FilesystemTest, testIsRelativePath) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysIsRelativePath("./foo"));
#ifdef WIN32
	EXPECT_FALSE(fs.sysIsRelativePath("C:"));
#endif
	EXPECT_TRUE(fs.sysIsRelativePath("foo"));
	EXPECT_TRUE(fs.sysIsRelativePath("foo/bar"));
	EXPECT_TRUE(fs.sysIsRelativePath("foo/bar/"));
	EXPECT_FALSE(fs.sysIsRelativePath("/foo"));
	EXPECT_FALSE(fs.sysIsRelativePath("/foo/bar"));
	EXPECT_FALSE(fs.sysIsRelativePath("/foo/bar/"));
	fs.shutdown();
}

TEST_F(FilesystemTest, testIsReadableDir) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysIsReadableDir(fs.homePath().str())) << fs.homePath().c_str() << " is not readable";
	fs.shutdown();
}

TEST_F(FilesystemTest, testListFilter) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysCreateDir(core::Path("listdirtestfilter")));
	EXPECT_TRUE(fs.sysCreateDir(core::Path("listdirtestfilter/dirxyz")));
	EXPECT_TRUE(fs.sysWrite("listdirtestfilter/filexyz", "1"));
	EXPECT_TRUE(fs.sysWrite("listdirtestfilter/fileother", "2"));
	EXPECT_TRUE(fs.sysWrite("listdirtestfilter/fileignore", "3"));
	core::DynamicArray<io::FilesystemEntry> entities;
	fs.list("listdirtestfilter/", entities, "*xyz");
	EXPECT_EQ(2u, entities.size()) << entities;
	ASSERT_FALSE(entities.empty()) << "Could not find any match";
	EXPECT_EQ(io::FilesystemEntry::Type::dir, entities[0].type);
	EXPECT_EQ(io::FilesystemEntry::Type::file, entities[1].type);
	fs.shutdown();
}

TEST_F(FilesystemTest, testMkdir) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysCreateDir(core::Path("testdir")));
	EXPECT_TRUE(fs.sysCreateDir(core::Path("testdir2/subdir/other")));
	EXPECT_TRUE(fs.sysRemoveDir("testdir2/subdir/other"));
	EXPECT_TRUE(fs.sysRemoveDir("testdir2/subdir"));
	EXPECT_TRUE(fs.sysRemoveDir("testdir2"));
	fs.shutdown();
}

TEST_F(FilesystemTest, testPushPopDir) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysCreateDir(core::Path("testdir")));
	EXPECT_TRUE(fs.sysPushDir(core::Path("testdir")));
	EXPECT_TRUE(fs.sysPopDir());
	fs.shutdown();
}

TEST_F(FilesystemTest, testWriteExplicitCurDir) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.homeWrite("./testfile", "123")) << "Failed to write content to testfile in " << fs.homePath().c_str();
	const core::String &content = fs.load("./testfile");
	EXPECT_EQ("123", content) << "Written content doesn't match expected";
	fs.shutdown();
}

TEST_F(FilesystemTest, testWrite) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.homeWrite("testfile", "123")) << "Failed to write content to testfile in " << fs.homePath().c_str();
	const core::String &content = fs.load("testfile");
	EXPECT_EQ("123", content) << "Written content doesn't match expected";
	fs.shutdown();
}

TEST_F(FilesystemTest, testWriteNewDir) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.homeWrite("dir123/testfile", "123")) << "Failed to write content to testfile in dir123";
	core::String filename;
	core::String filepath;
	core::String content;
	{
		const io::FilePtr& file = fs.open("dir123/testfile");
		filename = file->name();
		filepath = file->dir().str();
		content = file->load();
		file->close();
	}
	EXPECT_EQ("123", content) << "Written content doesn't match expected";
	EXPECT_TRUE(fs.sysRemoveFile(filename)) << "Failed to delete " << filename.c_str();
	EXPECT_TRUE(fs.sysRemoveDir(filepath)) << "Failed to delete " << filepath.c_str();
	fs.shutdown();
}

TEST_F(FilesystemTest, testCreateDirRecursive) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_TRUE(fs.sysCreateDir(core::Path("dir1/dir2/dir3/dir4"), true));
	EXPECT_TRUE(fs.sysRemoveDir("dir1/dir2/dir3/dir4"));
	EXPECT_TRUE(fs.sysRemoveDir("dir1/dir2/dir3"));
	EXPECT_TRUE(fs.sysRemoveDir("dir1/dir2"));
	EXPECT_TRUE(fs.sysRemoveDir("dir1"));
	fs.shutdown();
}

TEST_F(FilesystemTest, testCreateDirNonRecursiveFail) {
	io::Filesystem fs;
	EXPECT_TRUE(fs.init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_FALSE(fs.sysCreateDir(core::Path("does/not/exist"), false));
	fs.shutdown();
}

TEST_F(FilesystemTest, testSearchPathFor) {
	io::FilesystemPtr fs = core::make_shared<io::Filesystem>();
	EXPECT_TRUE(fs->init("test", "test")) << "Failed to initialize the filesystem";
	EXPECT_EQ(core::string::path(fs->sysCurrentDir().str(), "iotest.txt"), searchPathFor(fs, "foobar/does/not/exist", "iotest.txt"));
	ASSERT_TRUE(fs->sysWrite("dir123/testfile", "123")) << "Failed to write content to testfile in dir123";
	EXPECT_EQ(core::string::path(fs->sysCurrentDir().str(), "dir123/testfile"), searchPathFor(fs, "/foobar/does/not/dir123", "TestFile"));
	fs->shutdown();
}

} // namespace io
