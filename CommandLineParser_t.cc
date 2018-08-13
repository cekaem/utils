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
  parser.addBinaryParameter('d');
  try {
    parser.addIntegerParameter('d', 0);
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
    parser.parse(2, params);
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
  parser.addBinaryParameter('a');
  const char* params[] = {"./prog", "-a"};
  parser.parse(2, params);
  try {
    parser.getBinaryValue('b');
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
  parser.addBinaryParameter('a');
  parser.addStringParameter('s', "default_value", true);
  const char* params[] = {"./prog", "-a"};
  try {
    parser.parse(2, params);
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
  parser.addBinaryParameter('a');
  parser.addStringParameter('s', "default_value");
  const char* params[] = {"./prog", "-a"};
  try {
    parser.parse(2, params);
  } catch (CommandLineParser::CommandLineParserMandatoryParameterNotSetException& e) {
    NOT_REACHED
  }
  TEST_END
}

TEST_PROCEDURE(BadParameterTypeIsDetected) {
  TEST_START
  CommandLineParser parser;
  parser.addBinaryParameter('a');
  try {
    parser.getIntegerValue('a');
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
  parser.addIntegerParameter('a');
  const char* params[] = {"./prog", "-a"};
  try {
    parser.parse(2, params);
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
  parser.addFloatParameter('a');
  const char* params[] = {"./prog", "-a"};
  try {
    parser.parse(2, params);
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
  parser.addStringParameter('a');
  const char* params[] = {"./prog", "-a"};
  try {
    parser.parse(2, params);
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
  parser.addBinaryParameter('a');
  parser.addBinaryParameter('b');
  const char* params[] = {"./prog", "-a", "some_text", "-b"};
  try {
    parser.parse(4, params);
  } catch (CommandLineParser::CommandLineParserParseError& e) {
    VERIFY(string(e.str) == "some_text");
    RETURN;
  }
  NOT_REACHED
  TEST_END
}

TEST_PROCEDURE(getLastParameterStoresProperValue) {
  TEST_START
  CommandLineParser parser;
  parser.addBinaryParameter('a');
  const char* params[] = {"./prog", "-a", "some_text"};
  parser.parse(3, params);
  VERIFY(parser.getLastParameter() == "some_text");
  TEST_END
}

TEST_PROCEDURE(LackOfMandatoryLastParameterIsDetected) {
  TEST_START
  CommandLineParser parser;
  parser.setLastParameterIsMandatory();
  parser.addBinaryParameter('a');
  const char* params[] = {"./prog", "-a"};
  try {
    parser.parse(2, params);
  }  catch (CommandLineParser::CommandLineParserMandatoryParameterNotSetException& e) {
    VERIFY(e.parameter == ' ');
    RETURN
  }
  NOT_REACHED
  TEST_END
}

TEST_PROCEDURE(ParserDetectsPresenceOfLastParameterWhenItIsNotAllowed) {
  TEST_START
  {
    CommandLineParser parser;
    const char* params[] = {"./prog", "last_parameter"};
    parser.parse(2, params);
    VERIFY_STRINGS_EQUAL(parser.getLastParameter().c_str(), "last_parameter");
  }
  {
    CommandLineParser parser;
    parser.setLastParameterIsNotAllowed();
    const char* params[] = {"./prog", "last_parameter"};
    try {
      parser.parse(2, params);
    } catch (const CommandLineParser::CommandLineParserUnknownParameterException&) {
      RETURN
    }
    NOT_REACHED
  }
  TEST_END
}

TEST_PROCEDURE(TestOfExamplaryComplexCommandLine) {
  TEST_START
  CommandLineParser parser;

  parser.addIntegerParameter('a', 20);
  parser.addFloatParameter('b', 6.4f);
  parser.addStringParameter('c', "some_default_string");
  parser.addBinaryParameter('d');
  parser.addIntegerParameter('e', 5);
  parser.addFloatParameter('f', 40.3f);
  parser.addStringParameter('g', "default_string");
  parser.addBinaryParameter('h');
  
  const char* params[] = {"./prog", "-a", "10", "-b", "5.1", "-c", "some_string", "-d", "last_string"};
  parser.parse(9, params);

  VERIFY(parser.getIntegerValue('a') == 10);
  VERIFY(static_cast<float>(parser.getFloatValue('b')) == 5.1f);
  VERIFY(parser.getStringValue('c') == "some_string");
  VERIFY(parser.getBinaryValue('d') == true);
  VERIFY(parser.getIntegerValue('e') == 5);
  VERIFY(static_cast<float>(parser.getFloatValue('f')) == 40.3f);
  VERIFY(parser.getStringValue('g') == "default_string");
  VERIFY(parser.getBinaryValue('h') == false);
  VERIFY(parser.getLastParameter() == "last_string");

  TEST_END
}

} // unnamed namespace
