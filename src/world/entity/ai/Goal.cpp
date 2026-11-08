#include "world/entity/ai/Goal.h"

#include <algorithm>

bool GoalSelector::isRunning(Entry *entry) const
{
	return std::find(running.begin(), running.end(), entry) != running.end();
}

void GoalSelector::addGoal(int_t priority, std::unique_ptr<Goal> goal)
{
	auto entry = std::make_unique<Entry>();
	entry->priority = priority;
	entry->goal = std::move(goal);
	goals.push_back(std::move(entry));
}

void GoalSelector::removeGoal(Goal *goal)
{
	for (auto it = goals.begin(); it != goals.end();)
	{
		Entry *entry = it->get();
		if (entry->goal.get() == goal)
		{
			auto r = std::find(running.begin(), running.end(), entry);
			if (r != running.end())
			{
				goal->stop();
				running.erase(r);
			}
			it = goals.erase(it);
		}
		else
			++it;
	}
}

void GoalSelector::tick()
{
	std::vector<Entry *> toStart;
	if (tickCount++ % newGoalRate == 0)
	{
		for (auto &owned : goals)
		{
			Entry *entry = owned.get();
			if (isRunning(entry))
			{
				if (canUseInSystem(entry) && entry->goal->canContinueToUse())
					continue;
				entry->goal->stop();
				running.erase(std::find(running.begin(), running.end(), entry));
			}
			if (canUseInSystem(entry) && entry->goal->canUse())
			{
				toStart.push_back(entry);
				running.push_back(entry);
			}
		}
	}
	else
	{
		for (auto it = running.begin(); it != running.end();)
		{
			if (!(*it)->goal->canContinueToUse())
			{
				(*it)->goal->stop();
				it = running.erase(it);
			}
			else
				++it;
		}
	}

	for (Entry *entry : toStart)
		entry->goal->start();
	for (size_t i = 0; i < running.size(); ++i)
		running[i]->goal->tick();
}

bool GoalSelector::canUseInSystem(Entry *entry)
{
	for (auto &owned : goals)
	{
		Entry *other = owned.get();
		if (other == entry)
			continue;
		if (entry->priority >= other->priority)
		{
			if (isRunning(other) && !canCoExist(entry, other))
				return false;
		}
		else if (isRunning(other) && !other->goal->isInterruptable())
			return false;
	}
	return true;
}
