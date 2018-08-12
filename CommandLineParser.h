#ifndef COMMAND_LINE_PARSER_H
#define COMMAND_LINE_PARSER_H

#include <string>
#include <vector>


class CommandLineParser {
public:
  enum class ParameterType {
    BINARY,
    INTEGER,
    FLOAT,
    STRING
  };

  class CommandLineParserGeneralException {};

  // This exception is thrown when there is an attempt of adding already added parameter
  struct CommandLineParserParameterExistsException :
    public CommandLineParserGeneralException {
    CommandLineParserParameterExistsException(char p) : parameter(p) {}
    const char parameter;
  };

  /* This exception is thrown when parser finds unknown parameter in command line
   * (unknown means it was not added to parser by method add*Parameter) or when
   * user tries to get value for parameter which has not been added (by one of methods add*Parameter).
   */
  struct CommandLineParserUnknownParameterException :
    public CommandLineParserGeneralException {
    CommandLineParserUnknownParameterException(char p) : parameter(p) {}
    const char parameter;
  };

  /* This exception is thrown by parser when one of the mandatory parameters is not found in command
   * line parameters.
   */
  struct CommandLineParserMandatoryParameterNotSetException :
    public CommandLineParserGeneralException {
    CommandLineParserMandatoryParameterNotSetException(char p) : parameter(p) {}
    const char parameter;
  };

  /* This exception is thrown when someone tries to get parameter's value of another
   * type it was defined by method add*Parameter.
   */
  struct CommandLineParserBadParameterTypeException :
    public CommandLineParserGeneralException {
    CommandLineParserBadParameterTypeException(char p, ParameterType e, ParameterType f) :
      parameter(p),
      expected(e),
      found(f)
    {}
    const char parameter;
    const ParameterType expected;
    const ParameterType found;
  };

  /* This exception is thrown when there is a parameter in the parsed command line but it lacks
   * the value.
   */
  struct CommandLineParserParameterNeedsValueException :
    public CommandLineParserGeneralException {
    CommandLineParserParameterNeedsValueException(char p) : parameter(p) {}
    const char parameter;
  };

  /* This exception is thrown when there is a parse error in parsed command line.
   */
  struct CommandLineParserParseError :
    public CommandLineParserGeneralException {
    CommandLineParserParseError(const char* s) : str(s) {}
    const char* str;
  };

  CommandLineParser() : last_parameter_mandatory_(false) {}
  ~CommandLineParser();
  void addBinaryParameter(char parameter,
                          bool mandatory = false);
  void addIntegerParameter(char parameter,
                           int default_value = 0,
                           bool mandatory = false);
  void addFloatParameter(char parameter,
                         double default_value = 0.0f,
                         bool mandatory = false);
  void addStringParameter(char parameter,
                          const std::string& default_value = "",
                          bool mandatory = false);

  bool getBinaryValue(char parameter);
  const std::string& getStringValue(char parameter);
  int getIntegerValue(char parameter);
  double getFloatValue(char parameter);
  void setLastParameterIsMandatory() { last_parameter_mandatory_ = true; }
  const std::string& getLastParameter() { return last_parameter_; }
  void parse(int argc, const char** argv);

private:
  struct Parameter
  {
    Parameter(char p, bool m, ParameterType t) :
      parameter(p),
      mandatory(m),
      is_set(false),
      type(t) {}
    virtual ~Parameter() {}
    const char parameter;
    const bool mandatory;
    bool is_set;
    ParameterType type;
  };

  struct BinaryParameter : public Parameter
  {
    BinaryParameter(char parameter, bool mandatory) :
      Parameter(parameter, mandatory, ParameterType::BINARY),
      value(false)
    {}
    bool value;
  };

  struct IntegerParameter : public Parameter
  {
    IntegerParameter(char parameter, bool mandatory, int default_value) :
      Parameter(parameter, mandatory, ParameterType::INTEGER),
      value(default_value)
    {}
    int value;
  };

  struct FloatParameter : public Parameter
  {
    FloatParameter(char parameter, bool mandatory, double default_value) :
      Parameter(parameter, mandatory, ParameterType::FLOAT),
      value(default_value)
    {}
    double value;
  };

  struct StringParameter : public Parameter
  {
    StringParameter(char parameter, bool mandatory, const std::string& default_value) :
      Parameter(parameter, mandatory, ParameterType::STRING),
      value(default_value)
    {}
    ~StringParameter() { value.clear(); }
    std::string value;
  };

  void CheckIfDoesNotExist(char parameter);
  Parameter* getParameter(char parameter);
  Parameter* getParameter(char parameter, ParameterType type);
  bool IsLetter(char c);

  std::vector<Parameter*> parameters_;
  std::string last_parameter_;
  bool last_parameter_mandatory_;
};


#endif // COMMAND_LINE_PARSER_H
