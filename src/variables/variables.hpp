/*
 * variables.hpp
 *
 * Created on: 1 Jul 2011
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
 *
 * This file is part of Schnek.
 *
 * Schnek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Schnek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCHNEK_VARIABLES_HPP_
#define SCHNEK_VARIABLES_HPP_

#include <list>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>

#include "types.hpp"
#include "../exception.hpp"

namespace schnek {

class BlockVariables;
typedef boost::shared_ptr<BlockVariables> pBlockVariables;
typedef std::list<pBlockVariables> BlockVariablesList;

/** This stores a variable to be used in the simulation.
 *
 * A variable can be either a fixed value or an expression. The two types ValueVariant
 * and ExpressionType are boost::variants that can contain the variables or expressions
 * of different return types.
 */
class Variable
{
  public:
    /// Enum specifying which type the variable returns
    enum VariableTypeInfo {int_type, float_type, string_type};

    typedef ExpressionVariant ExpressionType;

  private:
    /// A boost::variant that holds expressions with different return type
    ExpressionType expression;
    /// A boost::variant that holds the basic constant values
    ValueVariant var;
    /// Holds the information about the type
    VariableTypeInfo type;
    /// fixed is true if a value is stored and false if an expression is stored in the variable
    bool fixed;
    /// Is set to true if the variable has been initialised
    bool initialised;
  public:
    /// construct with an integer
    Variable(int value, bool initialised_ = true);
    /// construct with a float
    Variable(double value, bool initialised_ = true);
    /// construct with a string
    Variable(std::string value, bool initialised_ = true);
    /// construct with an integer expression
    Variable(pIntExpression expr, bool initialised_ = true);
    /// construct with an float expression
    Variable(pFloatExpression expr, bool initialised_ = true);
    /// construct with an string expression
    Variable(pStringExpression expr, bool initialised_ = true);

    /** copy constructor
     *  plain variables are copied by value, but expressions are shallow-copied
     */
    Variable(const Variable &var);

    /// assignment operator
    Variable &operator=(const Variable &var);

    /// returns the type of the variable
    VariableTypeInfo getType() {return type;}
    /// returns the fixed value of the variable
    ValueVariant &getValue() {return var;}
    /// returns the expression kept in the variable
    const ExpressionVariant &getExpression() {return expression;}
    /// returns the expression kept in the variable
    ValueVariant evaluateExpression();

    /// returns true if the value of the variable is constant and does not depend on non-constant variables
    bool isConstant() {return fixed;}

    /// returns true if the value of the variable has been initialised
    bool isInitialised() {return initialised;}
};

typedef boost::shared_ptr<Variable> pVariable;
typedef std::list<pVariable> VariableList;
typedef std::map<std::string, pVariable> VariableMap;

/** Contains all the variables within a block together and is part of the block hierarchy
 *
 * The children are those blocks contained within the block, the parent is the surrounding block.
 * When searching for a variable, first the block itself is searched, then the parent blocks
 * successively all the way to the root.
 *
 * When a query contains a . the name is expanded to a block path
 */
class BlockVariables
{
  private:
    /// the unique name of this block
    std::string name;
    /// the classname of this block
    std::string className;
    /// the surrounding parent block
    pBlockVariables parent;
    /// The children of the block
    BlockVariablesList children;

    /// contains the actual variables of the block indexed by name
    VariableMap vars;

    typedef std::map<std::string, pBlockVariables> BlockVariablesMap;
    typedef std::map<std::string, BlockVariablesList> BlockVariablesListMap;
    BlockVariablesMap childrenByName;
    BlockVariablesListMap childrenByClassName;

    /** returns true if the variable exists
     * path is an expanded path name to the variable
     */
    bool exists(std::list<std::string> path, bool upward);

    /** Returns the variable specified by the path.
     * If the variable is not found, a fixed integer variable with value one is returned.
     * Use exists() to find out if the name is valid.
     *
     * Uses the same search pattern as the exists method.
     */
    pVariable getVariable(std::list<std::string> path, bool upward);
  public:
    /// Costruct using the block's name and class name
    BlockVariables(std::string name_, std::string className_, pBlockVariables parent_)
        : name(name_), className(className_), parent(parent_)
    {}

    /// returns true if the variable exists
    bool exists(std::string name);

    /** Returns the variable specified by the name.
     * If the variable is not found, a fixed integer variable with value one is returned.
     *
     * Use exists() to find out if the name is valid
     */
    pVariable getVariable(std::string name);

    /** Adds a new variable to the current block
     *  Returns true on success and false if the variable name already exists
     */
    bool addVariable(std::string name, pVariable variable);

    /** Adds a new child block
     *  Returns true on success and false if the block name already exists
     */
    bool addChild(pBlockVariables child);

    /// returns the block's name
    std::string getBlockName() { return name; }
    /// returns the block's class name
    std::string getClassName() { return className; }
    /// returns the parent (enclosing) block of this block
    pBlockVariables getParent() { return parent; }
    /// returns the children of the block
    const BlockVariablesList& getChildren() { return children; }

    /// returns the variables stored in the block
    const VariableMap& getVariables() { return vars; }
};

/** VariableStorage holds the blocks and variables in a tree structure
 *  The class contains a cursor that will follow the last inserted block. The
 *  cursor is used when constructing the structure from the input deck.
 */
class VariableStorage
{
  private:
    /// The root block
    pBlockVariables root;
    /// The current block
    pBlockVariables cursor;
  public:
    VariableStorage() {}
    /// Construct with the name and classname of the root block
    VariableStorage(std::string name, std::string classname);
    VariableStorage(const VariableStorage& storage)
      : root(storage.root), cursor(storage.cursor) {}

    /// reset the cursor to the root block
    void resetCursor();

    /** Move the cursor up one level.
     *  If the cursor is already at the root, nothing happens
     */
    void cursorUp();

    /** Add a new variable with a given name
     * The variable is inserted into the current block given
     * by the cursor.
     * Throws an exception if the variable name already exists.
     */
    void addVariable(std::string name, pVariable variable);

    /** Create a new block with the name and classname
     * The block is inserted as the child of the current block given
     * by the cursor.
     * Returns a pointer to the new block or throws an exception if the
     * block name already exists.
     */
    pBlockVariables createBlock(std::string name, std::string classname);

    /// Returns the root block
    pBlockVariables getRootBlock() { return root; }
    /// Returns the current block given by the cursor
    pBlockVariables getCurrentBlock() { return cursor; }
};

} // namespace

#endif // SCHNEK_VARIABLES_HPP_
