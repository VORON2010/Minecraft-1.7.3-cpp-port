#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"

// 1.5.2 Goal: one behaviour of a mob. The flags say which controls it uses
// (1 move, 2 look, 4 jump); goals that share a flag cannot run together.
class Goal
{
	int_t flags = 0;

public:
	virtual ~Goal() {}

	virtual bool canUse() = 0;
	virtual bool canContinueToUse() { return canUse(); }
	virtual bool isInterruptable() { return true; }
	virtual void start() {}
	virtual void stop() {}
	virtual void tick() {}

	void setFlags(int_t value) { flags = value; }
	int_t getFlags() const { return flags; }
};

// 1.5.2 GoalSelector: every third tick re-checks all goals by priority, otherwise only
// asks the running ones whether they want to go on
class GoalSelector
{
	struct Entry
	{
		int_t priority;
		std::unique_ptr<Goal> goal;
	};

	std::vector<std::unique_ptr<Entry>> goals;
	std::vector<Entry *> running;
	int_t tickCount = 0;
	int_t newGoalRate = 3;

	bool isRunning(Entry *entry) const;
	bool canUseInSystem(Entry *entry);
	static bool canCoExist(Entry *a, Entry *b) { return (a->goal->getFlags() & b->goal->getFlags()) == 0; }

public:
	void addGoal(int_t priority, std::unique_ptr<Goal> goal);
	void removeGoal(Goal *goal);
	void tick();
};
