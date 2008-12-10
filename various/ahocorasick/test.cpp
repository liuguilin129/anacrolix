#include "ahocorasick.hpp"

#include <set>
#include <string>
#include <vector>

#include <boost/assign/list_of.hpp>
#include <boost/assign/ptr_list_of.hpp>

using namespace std;
using namespace boost::assign;
using namespace AC;

class CStrKeyword : public Keyword<char>
{
public:
	CStrKeyword(char const str[])
	:	string_(str),
		length_(strlen(str))
	{
	}
	virtual size_t size() const { return length_; }
	virtual char const &operator[](size_t index) const { return string_[index]; }
	virtual void debug_keyword() const { debug("\"%s\"", string_); }
	virtual void debug_symbol(size_t index) const { debug("\"%c\"", string_[index]); }
private:
	char const *string_;
	size_t length_;
};

class CStrHaystack : public Haystack<char>
{
public:
	CStrHaystack(char const hay[])
	:
		hay_(hay)
	{}

	virtual bool at(size_t offset, char const *&input) const
	{
		if (offset < strlen(hay_)) {
			input = &hay_[offset];
			return true;
		}
		else
			return false;
	}
private:
	char const *hay_;
};

static void print_hit(size_t index, Keyword<char> const *keyword)
{
	debug("Found hit [%zu]: ", index);
	keyword->debug_keyword();
	debug("\n");
}

bool test1()
{
	vector<Keyword<char> *> kw;
	kw.push_back(new CStrKeyword("he"));
	kw.push_back(new CStrKeyword("she"));
	kw.push_back(new CStrKeyword("his"));
	kw.push_back(new CStrKeyword("hers"));

	CStrHaystack hay("ushers");

	AhoCorasick<char> ac(kw.begin(), kw.end());
	ac(hay, print_hit);
	ac(hay, print_hit);

	return false;
}

bool test2()
{
	return false;
}

int main()
{
	test1();
	test2();

	return 0;
}