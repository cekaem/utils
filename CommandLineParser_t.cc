#include "CommandLineParser.h"
#include "Test.h"

#include <cmath>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

using namespace std;

namespace {

TEST_PROCEDURE(CommandLineParserParameterExistsExceptionIsThrown) {
	TEST_START
	CommandLineParser parser;
	parser.AddBinaryParameter('d');
	try {
		parser.AddIntegerParameter('d', 0);
	} catch (CommandLineParser::CommandLineParserParameterExistsException& e) {
		VERIFY(e.parameter == 'd');
		RETURN
	}
  NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(UnknownParameterInCommandLineIsDetected) {
	TEST_START
	CommandLineParser parser;
	const char* params[] = {"./prog", "-a"};
	try {
		parser.Parse(2, params);
	} catch (CommandLineParser::CommandLineParserUnknownParameterException& e) {
		VERIFY(e.parameter == 'a');
		RETURN
	}
  NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(ParserThrowsCommandLineParserUnknownParameterException) {
	TEST_START
	CommandLineParser parser;
	parser.AddBinaryParameter('a');
	const char* params[] = {"./prog", "-a"};
	parser.Parse(2, params);
	try {
		parser.GetBinaryValue('b');
	} catch (CommandLineParser::CommandLineParserUnknownParameterException& e) {
		VERIFY(e.parameter == 'b');
		RETURN
	}
  NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(LackOfMandatoryParameterIsDetected)
{
	TEST_START
	CommandLineParser parser;
	parser.AddBinaryParameter('a');
	parser.AddStringParameter('s', "default_value", true);
	const char* params[] = {"./prog", "-a"};
	try {
		parser.Parse(2, params);
	} catch (CommandLineParser::CommandLineParserMandatoryParameterNotSetException& e) {
		VERIFY(e.parameter == 's');
		RETURN
	}
  NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(NotMandatoryParametersAreNotTreatedAsMandatory) {
	TEST_START
	CommandLineParser parser;
	parser.AddBinaryParameter('a');
	parser.AddStringParameter('s', "default_value");
	const char* params[] = {"./prog", "-a"};
	try {
		parser.Parse(2, params);
	} catch (CommandLineParser::CommandLineParserMandatoryParameterNotSetException& e) {
		NOT_REACHED
	}
	TEST_END
}

TEST_PROCEDURE(BadParameterTypeIsDetected) {
	TEST_START
	CommandLineParser parser;
	parser.AddBinaryParameter('a');
	try {
		parser.GetIntegerValue('a');
	} catch (CommandLineParser::CommandLineParserBadParameterTypeException& e) {
		VERIFY(e.parameter == 'a');
		RETURN
	}
  NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(ParameterProvidedWithoutRequiredIntegerValueIsDetected) {
	TEST_START
	CommandLineParser parser;
	parser.AddIntegerParameter('a');
	const char* params[] = {"./prog", "-a"};
	try {
		parser.Parse(2, params);
	} catch (CommandLineParser::CommandLineParserParameterNeedsValueException& e) {
		VERIFY(e.parameter == 'a');
		RETURN;
	}
	NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(ParameterProvidedWithoutRequiredFloatValueIsDetected) {
	TEST_START
	CommandLineParser parser;
	parser.AddFloatParameter('a');
	const char* params[] = {"./prog", "-a"};
	try {
		parser.Parse(2, params);
	} catch (CommandLineParser::CommandLineParserParameterNeedsValueException& e) {
		VERIFY(e.parameter == 'a');
		RETURN;
	}
	NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(ParameterProvidedWithoutRequiredStringValueIsDetected) {
	TEST_START
	CommandLineParser parser;
	parser.AddStringParameter('a');
	const char* params[] = {"./prog", "-a"};
	try {
		parser.Parse(2, params);
	}
	catch (CommandLineParser::CommandLineParserParameterNeedsValueException& e) {
		VERIFY(e.parameter == 'a');
		RETURN;
	}
  NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(ParseErrorInCommandLineIsDetected) {
	TEST_START
	CommandLineParser parser;
	parser.AddBinaryParameter('a');
	parser.AddBinaryParameter('b');
	const char* params[] = {"./prog", "-a", "some_text", "-b"};
	try {
		parser.Parse(4, params);
	} catch (CommandLineParser::CommandLineParserParseError& e) {
		VERIFY(string(e.str) == "some_text");
		RETURN;
	}
	NOT_REACHED
	TEST_END
}

TEST_PROCEDURE(GetLastParameterStoresProperValue) {
	TEST_START
	CommandLineParser parser;
	parser.AddBinaryParameter('a');
	const char* params[] = {"./prog", "-a", "some_text"};
	parser.Parse(3, params);
	VERIFY(parser.GetLastParameter() == "some_text");
	TEST_END
}

TEST_PROCEDURE(LackOfMandatoryLastParameterIsDetected) {
	TEST_START
	CommandLineParser parser;
	parser.SetLastParameterIsMandatory();
	parser.AddBinaryParameter('a');
	const char* params[] = {"./prog", "-a"};
	try {
		parser.Parse(2, params);
	}	catch (CommandLineParser::CommandLineParserMandatoryParameterNotSetException& e) {
		VERIFY(e.parameter == ' ');
		RETURN
	}
	NOT_REACHED
	TEST_END
}


TEST_PROCEDURE(TestOfExamplaryComplexCommandLine) {
	TEST_START
	CommandLineParser parser;

	parser.AddIntegerParameter('a', 20);
	parser.AddFloatParameter('b', 6.4f);
	parser.AddStringParameter('c', "some_default_string");
	parser.AddBinaryParameter('d');
	parser.AddIntegerParameter('e', 5);
	parser.AddFloatParameter('f', 40.3f);
	parser.AddStringParameter('g', "default_string");
	parser.AddBinaryParameter('h');
	
	const char* params[] = {"./prog", "-a", "10", "-b", "5.1", "-c", "some_string", "-d", "last_string"};
	parser.Parse(9, params);

	VERIFY(parser.GetIntegerValue('a') == 10);
	VERIFY(static_cast<float>(parser.GetFloatValue('b')) == 5.1f);
	VERIFY(parser.GetStringValue('c') == "some_string");
	VERIFY(parser.GetBinaryValue('d') == true);
	VERIFY(parser.GetIntegerValue('e') == 5);
	VERIFY(static_cast<float>(parser.GetFloatValue('f')) == 40.3f);
	VERIFY(parser.GetStringValue('g') == "default_string");
	VERIFY(parser.GetBinaryValue('h') == false);
	VERIFY(parser.GetLastParameter() == "last_string");

	TEST_END
}

} // unnamed namespace
