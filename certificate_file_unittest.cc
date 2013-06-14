// Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/certificate_file.h"

#include <string>
#include <vector>

#include <base/file_path.h>
#include <base/file_util.h>
#include <base/files/scoped_temp_dir.h>
#include <base/stringprintf.h>
#include <gtest/gtest.h>

#include "shill/mock_glib.h"

using base::FilePath;
using base::StringPrintf;
using std::string;
using std::vector;
using testing::_;
using testing::Return;
using testing::SetArgumentPointee;
using testing::StrEq;

namespace shill {

class CertificateFileTest : public testing::Test {
 public:
  CertificateFileTest() : certificate_file_(&glib_) {}

  virtual void SetUp() {
    CHECK(temp_dir_.CreateUniqueTempDir());
    certificate_directory_ = temp_dir_.path().Append("certificates");
    certificate_file_.set_root_directory(certificate_directory_);
  }

 protected:
  static const char kDERData[];
  static const char kPEMData[];

  string ExtractHexData(const std::string &pem_data) {
    return CertificateFile::ExtractHexData(pem_data);
  }
  const FilePath &GetOutputFile() { return certificate_file_.output_file_; }
  const FilePath &GetRootDirectory() {
    return certificate_file_.root_directory_;
  }
  const char *GetPEMHeader() { return CertificateFile::kPEMHeader; }
  const char *GetPEMFooter() { return CertificateFile::kPEMFooter; }

  CertificateFile certificate_file_;
  MockGLib glib_;
  base::ScopedTempDir temp_dir_;
  base::FilePath certificate_directory_;
};

const char CertificateFileTest::kDERData[] =
    "This does not have to be a real certificate "
    "since we are not testing its validity.";
const char CertificateFileTest::kPEMData[] =
    "VGhpcyBkb2VzIG5vdCBoYXZlIHRvIGJlIGEgcmVhbCBjZXJ0aWZpY2F0ZSBzaW5j\n"
    "ZSB3ZSBhcmUgbm90IHRlc3RpbmcgaXRzIHZhbGlkaXR5Lgo=\n";


TEST_F(CertificateFileTest, Construction) {
  EXPECT_TRUE(GetRootDirectory() == certificate_directory_);
  EXPECT_FALSE(file_util::PathExists(GetRootDirectory()));
  EXPECT_TRUE(GetOutputFile().empty());
}

TEST_F(CertificateFileTest, CreatePEMFromStrings) {
  // Create a formatted PEM file from the inner HEX data.
  const vector<string> kPEMVector0{ kPEMData };
  FilePath outfile0 = certificate_file_.CreatePEMFromStrings(kPEMVector0);
  EXPECT_FALSE(outfile0.empty());
  EXPECT_TRUE(file_util::PathExists(outfile0));
  EXPECT_TRUE(file_util::ContainsPath(certificate_directory_, outfile0));
  string file_string0;
  EXPECT_TRUE(file_util::ReadFileToString(outfile0, &file_string0));
  string expected_output0 = StringPrintf(
      "%s\n%s%s\n", GetPEMHeader(), kPEMData, GetPEMFooter());
  EXPECT_EQ(expected_output0, file_string0);

  // Create a formatted PEM file from formatted PEM.
  const vector<string> kPEMVector1{ expected_output0, kPEMData };
  FilePath outfile1 = certificate_file_.CreatePEMFromStrings(kPEMVector1);
  EXPECT_FALSE(outfile1.empty());
  EXPECT_TRUE(file_util::PathExists(outfile1));
  EXPECT_FALSE(file_util::PathExists(outfile0));  // Old file is deleted.
  string file_string1;
  EXPECT_TRUE(file_util::ReadFileToString(outfile1, &file_string1));
  string expected_output1 = StringPrintf(
      "%s%s", expected_output0.c_str(), expected_output0.c_str());
  EXPECT_EQ(expected_output1, file_string1);

  // Fail to create a PEM file.  Old file should not have been deleted.
  const vector<string> kPEMVector2{ kPEMData, "" };
  FilePath outfile2 = certificate_file_.CreatePEMFromStrings(kPEMVector2);
  EXPECT_TRUE(outfile2.empty());
  EXPECT_TRUE(file_util::PathExists(outfile1));
}

TEST_F(CertificateFileTest, CreateDERFromString) {
  // Create a DER file from the inner HEX data.
  const string kPEMString = kPEMData;
  const string fake_data("this is a fake");

  EXPECT_CALL(glib_, B64Decode(StrEq(kPEMString), _))
      .WillOnce(Return(false))
      .WillOnce(DoAll(SetArgumentPointee<1>(fake_data),
                      Return(true)));
  EXPECT_TRUE(certificate_file_.CreateDERFromString(kPEMData).empty());

  FilePath outfile = certificate_file_.CreateDERFromString(kPEMData);
  EXPECT_FALSE(outfile.empty());
  EXPECT_TRUE(file_util::PathExists(outfile));
  string file_string;
  EXPECT_TRUE(file_util::ReadFileToString(outfile, &file_string));
  EXPECT_EQ(fake_data, file_string);
}

TEST_F(CertificateFileTest, ExtractHexData) {
  EXPECT_EQ("", ExtractHexData(""));
  EXPECT_EQ("foo\n", ExtractHexData("foo"));
  EXPECT_EQ("foo\nbar\n", ExtractHexData("foo\r\n\t\n bar\n"));
  EXPECT_EQ("", ExtractHexData(
      StringPrintf("%s\nfoo\nbar\n%s\n", GetPEMFooter(), GetPEMHeader())));
  EXPECT_EQ("", ExtractHexData(
      StringPrintf("%s\nfoo\nbar\n%s\n", GetPEMHeader(), GetPEMHeader())));
  EXPECT_EQ("", ExtractHexData(
      StringPrintf("%s\nfoo\nbar\n", GetPEMHeader())));
  EXPECT_EQ("", ExtractHexData(
      StringPrintf("foo\nbar\n%s\n", GetPEMFooter())));
  EXPECT_EQ("foo\nbar\n", ExtractHexData(
      StringPrintf("%s\nfoo\nbar\n%s\n", GetPEMHeader(), GetPEMFooter())));
  EXPECT_EQ("bar\n", ExtractHexData(
      StringPrintf("foo\n%s\nbar\n%s\nbaz\n", GetPEMHeader(), GetPEMFooter())));
}

TEST_F(CertificateFileTest, Destruction) {
  FilePath outfile;
  {
    CertificateFile certificate_file(&glib_);
    certificate_file.set_root_directory(temp_dir_.path());
    outfile = certificate_file.CreatePEMFromStrings(vector<string>{ kPEMData });
    EXPECT_TRUE(file_util::PathExists(outfile));
  }
  // The output file should be deleted when certificate_file goes out-of-scope.
  EXPECT_FALSE(file_util::PathExists(outfile));
}

}  // namespace shill
