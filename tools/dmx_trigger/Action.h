/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Action.cpp
 * Copyright (C) 2011 Simon Newton
 */


#ifndef TOOLS_DMX_TRIGGER_ACTION_H_
#define TOOLS_DMX_TRIGGER_ACTION_H_

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdint.h>
#include <sstream>
#include <string>
#include <vector>
#include HASH_MAP_H

using std::string;


/**
 * A context is a collection of variables and their values.
 */
class Context {
  public:
    Context() {}
    ~Context();

    bool Lookup(const string &name, string *value) const;
    void Update(const string &name, const string &value);

    string AsString() const;
    friend std::ostream& operator<<(std::ostream &out, const Context&);

  private:
    typedef HASH_NAMESPACE::HASH_MAP_CLASS<string, string> VariableMap;
    VariableMap m_variables;
};


/*
 * An Action is a behavior that is run when a particular DMX value is received
 * on a particular slot.
 *
 * This is the base class that all others inherit from.
 */
class Action {
  public:
    Action()
      : m_ref_count(0) {
    }
    virtual ~Action() {}

    void Ref() {
      m_ref_count++;
    }
    void DeRef() {
      m_ref_count--;
      if (m_ref_count == 0)
        delete this;
    }
    virtual void Execute(Context *context, uint8_t slot_value) = 0;

  private:
    unsigned int m_ref_count;
};


/**
 * An Action that assigned a value to a variable
 */
class VariableAssignmentAction: public Action {
  public:
    VariableAssignmentAction(const string &variable,
                             const string &value)
        : Action(),
          m_variable(variable),
          m_value(value) {
    }

    void Execute(Context *context, uint8_t slot_value);

  private:
    const string m_variable;
    const string m_value;
};


/**
 * A shell command action
class ShellCommandAction(const string 


    void Execute();

  private:

};
 */



/**
 * An interval of DMX values and then action to be taken for matching values.
 */
class ActionInterval {
  public:
    ActionInterval(uint8_t lower, uint8_t upper, Action *action)
        : m_lower(lower),
          m_upper(upper),
          m_action(action) {
      if (m_action)
        m_action->Ref();
    }

    ~ActionInterval() {
      if (m_action)
        m_action->DeRef();
    }

    uint8_t Lower() const { return m_lower; }
    uint8_t Upper() const { return m_upper; }
    Action *GetAction() const { return m_action; }

    bool Contains(uint8_t value) const {
      return value >= m_lower && value <= m_upper;
    }

    bool Intersects(const ActionInterval &other) const {
      return (other.Contains(m_lower) || other.Contains(m_upper) ||
              Contains(other.m_lower) || Contains(other.m_upper));
    }

    bool operator<(const ActionInterval &other) const {
      return m_lower < other.m_lower;
    };

    string AsString() const;
    friend std::ostream& operator<<(std::ostream &out, const ActionInterval&);

  private:
    uint8_t m_lower, m_upper;
    Action *m_action;
};


/**
 * The set of intervals and their actions.
 */
class SlotActions {
  public:
    explicit SlotActions(uint16_t slot_offset)
      : m_default_action(NULL),
        m_slot_offset(slot_offset) {
    }
    ~SlotActions();

    bool AddAction(uint8_t lower, uint8_t upper, Action *action);
    bool SetDefaultAction(Action *action);
    void TakeAction(Context *context, uint8_t value);

    string IntervalsAsString() const;

  private:
    Action *m_default_action;
    uint16_t m_slot_offset;

    typedef std::vector<ActionInterval*> ActionVector;
    ActionVector m_actions;

    bool ValueWithinIntervals(uint8_t value,
                              const ActionInterval &lower_interval,
                              const ActionInterval &upper_interval);
    bool IntervalsIntersect(const ActionInterval *a1,
                            const ActionInterval *a2);
    ActionInterval *LocateMatchingAction(uint8_t value);
    string IntervalsAsString(const ActionVector::const_iterator &start,
                             const ActionVector::const_iterator &end) const;
};
#endif  // TOOLS_DMX_TRIGGER_ACTION_H_