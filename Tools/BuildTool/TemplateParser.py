import re
import os
from enum import Enum

TokenDictionary = {
    "UNKNOWN_PLATFORM" : 0,
    "QUAINT_PLATFORM_WIN32" : 1,
    "DEBUG_BUILD" : 1
}

class ParamType(Enum):
    EDictionary = 0
    EList = 1
    EMacro = 2
    ENumber = 3
    EString = 4
    EComma = 5
    EInvalid = 6

def IdentifyParamType(Param : str, Index) -> ParamType | None:
    if (Param is None) or (len(Param) == 0):
        return ParamType.EInvalid

    Type = ParamType.EInvalid
    assert (Index < len(Param)), "Invalid Index retrieved"

    c = Param[Index]
    if c == '{' : 
        Type = ParamType.EDictionary
    elif c == '[' or c == '(':
        Type = ParamType.EList
    elif c == '#':
        Type = ParamType.EMacro
    elif ord(c) >= 48 and ord(c) <= 57:
        Type = ParamType.ENumber
    elif c == "\"" or c == "\'":
        Type = ParamType.EString
    elif c == ",":
        Type = ParamType.EComma
    else:
        assert False, "Invalid Type Encountered"

    return Type

def EvaluateBooleanExpression(Tokens : list, Index) -> tuple[bool, int]:
    assert(Index < len(Tokens)), "Invalid Conditional Expression Encountered"

    ExprRes = True
    while(Index < len(Tokens)):
        if Tokens[Index] == "defined":
            assert(Index + 1 < len(Tokens)), "No Token Specified after 'defined' statement"
            Index += 1
            assert(Tokens[Index] != ')' and Tokens[Index] != 'and' and Tokens[Index] != 'or'), "Keyword encounterd at wrong location"
            if Tokens[Index] == '(':
                (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
                ExprRes &= Res
            else:
                ExprRes &= TokenDictionary.get(Tokens[Index]) != None
                Index += 1

        elif Tokens[Index] == "and":
            assert(Index + 1 < len(Tokens)), "No Token Specified after 'and' statement"
            (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
            ExprRes &= Res
        
        elif Tokens[Index] == "or":
            assert(Index + 1 < len(Tokens)), "No Token Specified after 'and' statement"
            (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
            ExprRes |= Res
        
        elif Tokens[Index] == "not":
            assert(Index + 1 < len(Tokens)), "No Token Specified after 'and' statement"
            (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
            ExprRes &= ~Res

        elif Tokens[Index] == '(':
            (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
            ExprRes &= Res

        elif Tokens[Index] == ')':
            return (ExprRes, Index)
        
        else: #Token is a variable to evaluate
            #TODO: Add Support for comparing two user defined tokens
            ExprRes &= TokenDictionary.get(Tokens[Index]) != None
            Index += 1

    return (ExprRes, Index)

def GetPreProcessorToken(Param, Index) -> tuple[str, int]:
    Index = GetNextValidCharacterIndex(Param, Index)
    PreProcessorToken = ""
    while(Index < len(Param)) and (Param[Index] != ' ' and Param[Index] != '\n' and Param[Index] != '\r'):
        PreProcessorToken += Param[Index]
        Index += 1
    return (PreProcessorToken, Index)

def PrasePreprocessorBlock(Param, Index, PreProcessorToken) -> tuple[str, int]:
    #(PreProcessorToken, Index) = GetPreProcessorToken(Param, Index)
    assert PreProcessorToken == "if" or \
            PreProcessorToken == "elif" or \
            PreProcessorToken == "else" or \
            PreProcessorToken == "endif", "Invalid PreProcessor Type encountered"
    
    #Accumulate Tokens until \n or \r is encountered
    if PreProcessorToken == "if" or PreProcessorToken == "elif":
        Index = GetNextValidCharacterIndex(Param, Index)
        BoolExpressionTokens = []
        while(Index < len(Param)) and Param[Index] != '\n' and Param[Index] != '\r':
            if Param[Index] == ' ' :
                Index = GetNextValidCharacterIndex(Param, Index)
            (Token, Index) = GetToken(Param, Index)
            if(Token != ""):
                BoolExpressionTokens.append(Token)


        (BoolRes, TupIndex) = EvaluateBooleanExpression(BoolExpressionTokens, 0)
        if BoolRes:
            return (Param, Index)        
        else:
            # Parent if is false. Skip any nested ifs and jump the control to #else or #endif
            IfStack = []
            while Param[Index] != '#' or len(IfStack) != 0:
                Index = GetNextValidCharacterIndex(Param, Index)
                if(Param[Index] == '#'):
                    (PreProcessorToken, Index) = GetPreProcessorToken(Param, Index)
                    if(PreProcessorToken == "if"):
                        IfStack.append(PreProcessorToken)
                    elif (PreProcessorToken == "endif") and len(IfStack) != 0:
                        IfStack.pop()
                    elif (PreProcessorToken == "else" or PreProcessorToken == "elif"):
                        if len(IfStack) == 0:
                            break
                    elif len(IfStack) == 0:
                        break
                    else:
                        assert False, "Entered Invalid State when parsing Preprocessor block"

            #We would've skipped all the nested ifs. If there's an elif, process that
            if PreProcessorToken == "elif":
                return PrasePreprocessorBlock(Param, Index, "elif")
            else:
                return(Param, Index)
            
    #If control reaches the 'else' block, skip it entirely. else block will be handled within if, elif 
    elif PreProcessorToken == "else":
        IfStack = ["if"]
        while(PreProcessorToken != "endif") and len(IfStack) != 0:
            while(Param[Index] != '#'):
                Index += 1
                assert(Index != len(Param)), "#endif not encountered"
            
            assert Param[Index] == '#', "Invalid Preprocessor block"
            (PreProcessorToken, Index) = GetPreProcessorToken(Param, Index)
            
            if(PreProcessorToken == "if"):
                IfStack.append("if")
            elif(PreProcessorToken == "endif"):
                IfStack.pop()    
        return(Param, Index)
    
    else:
        return(Param, Index)

    pass

def GetToken(Param, Index) -> tuple[str, int]:
    Token = ""
    assert Index < len(Param), "Invalid Index Passed"

    if Param[Index] == '(' or Param[Index] == ')':
        Token = Param[Index]
        Index += 1
        return Token

    while(Index < len(Param)) and Param[Index] != '\n' and Param[Index] != '\r' and Param[Index] != ' ':
        Token += Param[Index]
        Index += 1
    return (Token, Index)

def GetNextValidCharacterIndex(Param, Index) -> int:
    Index += 1
    while(Index < len(Param)) and (Param[Index] ==  ' ' or Param[Index] ==  '\n' or Param[Index] ==  '\r'):
        Index += 1
        continue
    return Index

def GetNextIndex(Param, Index) -> int:
    Index = GetNextValidCharacterIndex(Param, Index)
    if(Index == len(Param)): return Index

    if(Param[Index] == "#"):
        (Token, Index) = GetPreProcessorToken(Param, Index)
        (Param, Index) = PrasePreprocessorBlock(Param, Index, Token)
        Index = GetNextValidCharacterIndex(Param, Index)
    elif(Param[Index] == '/'):
        Index+=1
        assert(Index <= len(Param)), "Terminated unexpectedly"
        if(Param[Index] == '/'):
            while(Index < len(Param)) and (Param[Index] !=  '\n' or Param[Index] ==  '\r'):
                Index += 1
                continue
            Index = GetNextIndex(Param, Index)
    return Index

def ParseNumber(Param, Index) -> tuple[int | float, int]:
    assert ord(Param[Index]) >= 48 and ord(Param[Index]) <= 57

    NumRes = Param[Index]
    while(ord(Param[Index + 1]) >= 48 and ord(Param[Index + 1]) <= 57) or (Param[Index + 1] == '.'):
        NumRes += Param[Index + 1]
        Index += 1

    if(NumRes.count('.') != 0):
        return (float(NumRes), Index)
    return (int(NumRes), Index)

def ParseString(Param, Index) -> tuple[str, int]:
    assert Param[Index] == '\"' or Param[Index] == '\''
    
    StringRes = ""
    Index += 1

    while(Param[Index] != '\"' and Param[Index] != '\''):
        StringRes += Param[Index]
        Index += 1
    
    return (StringRes, Index)

def ParseList(Param, Index) -> tuple[list, int]:
    assert Param[Index] == '['
    
    ListRes = []
    Index = GetNextIndex(Param, Index)
    while Param[Index] != ']':
        (Res, Index) = ProcessParam(Param, Index)
        ListRes.append(Res)

        Index = GetNextIndex(Param, Index)
        assert Param[Index] == ',' or Param[Index] == ']'
        if Param[Index] == ',':
            Index = GetNextIndex(Param, Index)
        pass

    return (ListRes, Index)

def ParseDictionary(Param, Index) -> tuple[dict, int]:
    assert Param[Index] == '{'
    
    DictRes = {}
    Index = GetNextIndex(Param, Index)
    while Param[Index] != '}':

        Key = ""
        assert (Param[Index] == '\"' or Param[Index] == '\''), "Invalid Syntax. Key missing!"
        if Param[Index] == '\"' or Param[Index] == '\'':
            (Key, Index) = ParseString(Param, Index)
            pass

        Index = GetNextIndex(Param, Index)
        assert Param[Index] == ':', "Invalid Syntax. No ':' after Key"
        Index = GetNextIndex(Param, Index)

        (DictRes[Key], Index) = ProcessParam(Param, Index)

        Index = GetNextIndex(Param, Index)
        assert Param[Index] == ',' or Param[Index] == '}'
        if Param[Index] == ',':
            Index = GetNextIndex(Param, Index)

    return(DictRes, Index)


def ProcessParam(Param, Index) -> tuple[dict | list | str | None, int]:
    #Index = GetNextValidCharacterIndex(Param, Index)
    Type = IdentifyParamType(Param, Index)
    Res = {}
    if Type == ParamType.EDictionary:
        (Res, Index) = ParseDictionary(Param, Index)
    elif Type == ParamType.EList:
        (Res, Index) = ParseList(Param, Index)
    elif Type == ParamType.ENumber:
        (Res, Index) = ParseNumber(Param, Index)
    elif Type == ParamType.EString:
        (Res, Index) = ParseString(Param, Index)
    else:
        assert False, "Trying to parse invalid type"

    return (Res, Index)

def ParseBlock(Param, Index) -> dict:
    #Supports Assignments if line starts with a string
    ParamDictionary = {}
    Index = GetNextIndex(Param, Index)
    while(Index < len(Param)):
        Type = IdentifyParamType(Param, Index)
        if(Type == ParamType.EString):
            (KeyEntry, Index) = ParseString(Param, Index)
            Index = GetNextIndex(Param, Index)
            assert Param[Index] == '=', "Not a valid entry"
            Index = GetNextIndex(Param, Index)
            (ParamDictionary[KeyEntry], Index) = ProcessParam(Param, Index)
        elif(Type == ParamType.EComma):
            pass
        else:
            assert False, "Invalid Character encountered when parsing block"

        Index = GetNextIndex(Param, Index)

    
    return ParamDictionary
        

def ReadTemplateFile(TemplateFilePath):
    if not os.path.isfile(TemplateFilePath):
        print("Encountered something that's not a file when trying to read template file")
        return None
    
    stream = open(TemplateFilePath, "r")
    contents = stream.read()
    stream.close()

    Blocks = re.finditer("(@:)(.*(\n|\r|\r\n))+?(.*:@)", contents)

    if(Blocks == None):
        print("No Module params found for this Module. This will skip Parsing Template")
    
    ParamDictionary = {}
    for Params in Blocks:
        CleanedStr = re.sub("(@:)|(:@)", "", Params.group())
        ResDict = ParseBlock(CleanedStr, -1)
        ParamDictionary.update(ResDict)
        
    return ParamDictionary
