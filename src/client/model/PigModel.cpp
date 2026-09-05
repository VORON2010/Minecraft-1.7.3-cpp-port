#include "client/model/PigModel.h"

PigModel::PigModel() : PigModel(0.0f)
{
}

PigModel::PigModel(float grow) : QuadrupedModel(6, grow)
{
}
