//==========================================================================
// nedyyutil.cc  -
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2002-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <cassert>
#include "common/opp_ctype.h"
#include "common/stringutil.h"
#include "common/unitconversion.h"
#include "nedyyutil.h"
#include "sourcedocument.h"

using namespace omnetpp::common;

namespace omnetpp {
namespace nedxml {
namespace nedyyutil {  // for now

static NedAstNodeFactory factory;

ASTNode *createNedElementWithTag(ParseContext *np, int tagcode, ASTNode *parent)
{
    return createElementWithTag(np, &factory, tagcode, parent);
}

ASTNode *getOrCreateNedElementWithTag(ParseContext *np, int tagcode, ASTNode *parent)
{
    return getOrCreateElementWithTag(np, &factory, tagcode, parent);
}

//
// Properties
//

PropertyElement *addProperty(ParseContext *np, ASTNode *node, const char *name)
{
    PropertyElement *prop = (PropertyElement *)createNedElementWithTag(np, NED_PROPERTY, node);
    prop->setName(name);
    return prop;
}

PropertyElement *addComponentProperty(ParseContext *np, ASTNode *node, const char *name)
{
    // add propery under the ParametersElement; create that if not yet exists
    ASTNode *params = node->getFirstChildWithTag(NED_PARAMETERS);
    if (!params) {
        params = createNedElementWithTag(np, NED_PARAMETERS, node);

        // move parameters section in front of potential gates, types, etc sections
        ASTNode *prev;
        while ((prev = params->getPrevSibling()) != nullptr &&
               (prev->getTagCode() == NED_GATES || prev->getTagCode() == NED_TYPES ||
                prev->getTagCode() == NED_SUBMODULES || prev->getTagCode() == NED_CONNECTIONS))
        {
            node->removeChild(params);
            node->insertChildBefore(prev, params);
        }
    }
    PropertyElement *prop = (PropertyElement *)createNedElementWithTag(np, NED_PROPERTY, params);
    prop->setName(name);
    return prop;
}

//
// Spec Properties: source code, display string
//

PropertyElement *storeSourceCode(ParseContext *np, ASTNode *node, YYLoc tokenpos)
{
    PropertyElement *prop = addProperty(np, node, "sourcecode");
    prop->setIsImplicit(true);
    PropertyKeyElement *propkey = (PropertyKeyElement *)createNedElementWithTag(np, NED_PROPERTY_KEY, prop);
    propkey->appendChild(createLiteral(np, LIT_STRING, tokenpos, makeEmptyYYLoc()));  // don't store it twice
    return prop;
}

PropertyElement *storeComponentSourceCode(ParseContext *np, ASTNode *node, YYLoc tokenpos)
{
    PropertyElement *prop = addComponentProperty(np, node, "sourcecode");
    prop->setIsImplicit(true);
    PropertyKeyElement *propkey = (PropertyKeyElement *)createNedElementWithTag(np, NED_PROPERTY_KEY, prop);
    propkey->appendChild(createLiteral(np, LIT_STRING, tokenpos, makeEmptyYYLoc()));  // don't store it twice
    return prop;
}

PropertyElement *setIsNetworkProperty(ParseContext *np, ASTNode *node)
{
    PropertyElement *prop = addComponentProperty(np, node, "isNetwork");
    prop->setIsImplicit(true);
    // Note: the following is not needed, because @isNetwork already means @isNetwork(true)
    // PropertyKeyElement *propkey = (PropertyKeyElement *)createNedElementWithTag(NED_PROPERTY_KEY, prop);
    // propkey->appendChild(createLiteral(LIT_BOOL, "true", ""));
    return prop;
}

//
// Comments
//
void addComment(ParseContext *np, ASTNode *node, const char *locId, const char *text, const char *defaultValue)
{
    // don't store empty string or the default
    if (!text[0] || strcmp(text, defaultValue) == 0)
        return;

    CommentElement *comment = (CommentElement *)createNedElementWithTag(np, NED_COMMENT);
    comment->setLocid(locId);
    comment->setContent(text);
    node->insertChildBefore(node->getFirstChild(), comment);
}

void storeFileComment(ParseContext *np, ASTNode *node)
{
    addComment(np, node, "banner", np->getSource()->getFileComment(), "");
}

void storeBannerComment(ParseContext *np, ASTNode *node, YYLoc tokenpos)
{
    addComment(np, node, "banner", np->getSource()->getBannerComment(tokenpos), "");
}

void storeRightComment(ParseContext *np, ASTNode *node, YYLoc tokenpos)
{
    addComment(np, node, "right", np->getSource()->getTrailingComment(tokenpos), "\n");
}

void storeTrailingComment(ParseContext *np, ASTNode *node, YYLoc tokenpos)
{
    addComment(np, node, "trailing", np->getSource()->getTrailingComment(tokenpos), "\n");
}

void storeBannerAndRightComments(ParseContext *np, ASTNode *node, YYLoc pos)
{
    np->getSource()->trimSpaceAndComments(pos);
    storeBannerComment(np, node, pos);
    storeRightComment(np, node, pos);
    storeInnerComments(np, node, pos);
}

void storeBannerAndRightComments(ParseContext *np, ASTNode *node, YYLoc firstpos, YYLoc lastpos)
{
    YYLoc pos = firstpos;
    pos.last_line = lastpos.last_line;
    pos.last_column = lastpos.last_column;

    np->getSource()->trimSpaceAndComments(pos);
    storeBannerComment(np, node, pos);
    storeRightComment(np, node, pos);
}

void storeInnerComments(ParseContext *np, ASTNode *node, YYLoc pos)
{
    for (;;) {
        const char *comment = np->getSource()->getNextInnerComment(pos);  // updates "pos"
        if (!comment)
            break;
        addComment(np, node, "inner", comment, "");
    }
}

ParamElement *addParameter(ParseContext *np, ASTNode *params, YYLoc namepos)
{
    ParamElement *param = (ParamElement *)createNedElementWithTag(np, NED_PARAM, params);
    param->setName(toString(np, namepos));
    return param;
}

ParamElement *addParameter(ParseContext *np, ASTNode *params, const char *name, YYLoc namepos)
{
    ParamElement *param = (ParamElement *)createNedElementWithTag(np, NED_PARAM, params);
    param->setName(name);
    return param;
}

GateElement *addGate(ParseContext *np, ASTNode *gates, YYLoc namepos)
{
    GateElement *gate = (GateElement *)createNedElementWithTag(np, NED_GATE, gates);
    gate->setName(toString(np, namepos));
    return gate;
}

void addOptionalExpression(ParseContext *np, ASTNode *elem, const char *attrname, YYLoc exprpos, ASTNode *expr)
{
    if (!expr)
        elem->setAttribute(attrname, toString(np, exprpos));
    else
        addExpression(np, elem, attrname, exprpos, expr);
}

void addExpression(ParseContext *np, ASTNode *elem, const char *attrname, YYLoc exprpos, ASTNode *expr)
{
    if (np->parseexpr) {
        assert(expr);
        ((ExpressionElement *)expr)->setTarget(attrname);

        // in the DTD, whilespaces and expressions are at front, insert there
        ASTNode *insertPos = elem->getFirstChild();
        while (insertPos && (insertPos->getTagCode() == NED_COMMENT || insertPos->getTagCode() == NED_EXPRESSION))
            insertPos = insertPos->getNextSibling();
        if (!insertPos)
            elem->appendChild(expr);
        else
            elem->insertChildBefore(insertPos, expr);
    }
    else {
        elem->setAttribute(attrname, toString(np, exprpos));
    }
}

void swapConnection(ASTNode *conn)
{
    swapAttributes(conn, "src-module", "dest-module");
    swapAttributes(conn, "src-module-index", "dest-module-index");
    swapAttributes(conn, "src-gate", "dest-gate");
    swapAttributes(conn, "src-gate-index", "dest-gate-index");
    swapAttributes(conn, "src-gate-plusplus", "dest-gate-plusplus");
    swapAttributes(conn, "src-gate-subg", "dest-gate-subg");

    swapExpressionChildren(conn, "src-module-index", "dest-module-index");
    swapExpressionChildren(conn, "src-gate-index", "dest-gate-index");
}

void swapExpressionChildren(ASTNode *node, const char *attr1, const char *attr2)
{
    ExpressionElement *expr1, *expr2;
    for (expr1 = (ExpressionElement *)node->getFirstChildWithTag(NED_EXPRESSION); expr1; expr1 = expr1->getNextExpressionSibling())
        if (!strcmp(expr1->getTarget(), attr1))
            break;

    for (expr2 = (ExpressionElement *)node->getFirstChildWithTag(NED_EXPRESSION); expr2; expr2 = expr2->getNextExpressionSibling())
        if (!strcmp(expr2->getTarget(), attr2))
            break;


    if (expr1)
        expr1->setTarget(attr2);
    if (expr2)
        expr2->setTarget(attr1);
}

OperatorElement *createOperator(ParseContext *np, const char *op, ASTNode *operand1, ASTNode *operand2, ASTNode *operand3)
{
    OperatorElement *opnode = (OperatorElement *)createNedElementWithTag(np, NED_OPERATOR);
    opnode->setName(op);
    opnode->appendChild(operand1);
    if (operand2) opnode->appendChild(operand2);
    if (operand3) opnode->appendChild(operand3);
    return opnode;
}

FunctionElement *createFunction(ParseContext *np, const char *funcname, ASTNode *arg1, ASTNode *arg2, ASTNode *arg3, ASTNode *arg4, ASTNode *arg5, ASTNode *arg6, ASTNode *arg7, ASTNode *arg8, ASTNode *arg9, ASTNode *arg10)
{
    FunctionElement *funcnode = (FunctionElement *)createNedElementWithTag(np, NED_FUNCTION);
    funcnode->setName(funcname);
    if (arg1) funcnode->appendChild(arg1);
    if (arg2) funcnode->appendChild(arg2);
    if (arg3) funcnode->appendChild(arg3);
    if (arg4) funcnode->appendChild(arg4);
    if (arg5) funcnode->appendChild(arg5);
    if (arg6) funcnode->appendChild(arg6);
    if (arg7) funcnode->appendChild(arg7);
    if (arg8) funcnode->appendChild(arg8);
    if (arg9) funcnode->appendChild(arg9);
    if (arg10) funcnode->appendChild(arg10);
    return funcnode;
}

ExpressionElement *createExpression(ParseContext *np, ASTNode *expr)
{
    ExpressionElement *expression = (ExpressionElement *)createNedElementWithTag(np, NED_EXPRESSION);
    expression->appendChild(expr);
    return expression;
}

IdentElement *createIdent(ParseContext *np, YYLoc parampos)
{
    IdentElement *ident = (IdentElement *)createNedElementWithTag(np, NED_IDENT);
    ident->setName(toString(np, parampos));
    return ident;
}

IdentElement *createIdent(ParseContext *np, YYLoc parampos, YYLoc modulepos, ASTNode *moduleindexoperand)
{
    IdentElement *ident = (IdentElement *)createNedElementWithTag(np, NED_IDENT);
    ident->setName(toString(np, parampos));
    ident->setModule(toString(np, modulepos));
    if (moduleindexoperand)
        ident->appendChild(moduleindexoperand);
    return ident;
}

LiteralElement *createPropertyValue(ParseContext *np, YYLoc textpos)  // which is a spec or a string literal
{
    np->getSource()->trimSpaceAndComments(textpos);

    bool isString = false;
    try {
        const char *text = toString(np, textpos);
        while (opp_isspace(*text))
            text++;
        if (*text == '"') {
            const char *endp;
            opp_parsequotedstr(text, endp);
            if (*endp == '\0')
                isString = true;
        }
    }
    catch (std::exception& e) {  /*not string*/
    }
    if (isString)
        return createStringLiteral(np, textpos);
    else
        return createLiteral(np, LIT_SPEC, textpos, textpos);
}

LiteralElement *createLiteral(ParseContext *np, int type, YYLoc valuepos, YYLoc textpos)
{
    LiteralElement *c = (LiteralElement *)createNedElementWithTag(np, NED_LITERAL);
    c->setType(type);
    c->setValue(opp_trim(toString(np, valuepos)).c_str());
    c->setText(toString(np, textpos));
    return c;
}

LiteralElement *createLiteral(ParseContext *np, int type, const char *value, const char *text)
{
    LiteralElement *c = (LiteralElement *)createNedElementWithTag(np, NED_LITERAL);
    c->setType(type);
    c->setValue(value);
    c->setText(text);
    return c;
}

LiteralElement *createStringLiteral(ParseContext *np, YYLoc textpos)
{
    LiteralElement *c = (LiteralElement *)createNedElementWithTag(np, NED_LITERAL);
    c->setType(LIT_STRING);

    const char *text = toString(np, textpos);
    c->setText(text);

    try {
        std::string value = opp_parsequotedstr(text);
        c->setValue(value.c_str());
    }
    catch (std::exception& e) {
        np->error(e.what(), pos.li);
    }
    return c;
}

LiteralElement *createQuantityLiteral(ParseContext *np, YYLoc textpos)
{
    LiteralElement *c = (LiteralElement *)createNedElementWithTag(np, NED_LITERAL);
    c->setType(LIT_QUANTITY);

    const char *text = toString(np, textpos);
    c->setText(text);
    c->setValue(text);

    try {
        // validate syntax and unit compatibility ("5s 2kg")
        std::string unit;
        UnitConversion::parseQuantity(text, unit);
    }
    catch (std::exception& e) {
        np->error(e.what(), pos.li);
    }
    return c;
}

ASTNode *prependMinusSign(ParseContext *np, ASTNode *node)
{
    Assert(node->getTagCode() == NED_LITERAL);

    LiteralElement *literalNode = (LiteralElement *)node;

    // only numeric constants can be negative, string, bool, etc. cannot
    int type = literalNode->getType();
    if (type != LIT_INT && type != LIT_DOUBLE && type != LIT_QUANTITY) {
        char msg[140];
        sprintf(msg, "unary minus not accepted before '%.100s'", literalNode->getValue());
        np->error(msg, pos.li);
        return node;
    }

    // prepend the constant with a '-'
    char *buf = new char[strlen(literalNode->getValue())+2];
    buf[0] = '-';
    strcpy(buf+1, literalNode->getValue());
    literalNode->setValue(buf);
    literalNode->setText(buf);  // for numeric constants, text and value are the same string
    delete[] buf;

    return node;
}

}  // namespace nedyyutil
}  // namespace nedxml
}  // namespace omnetpp

