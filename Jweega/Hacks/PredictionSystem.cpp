#include "PredictionSystem.h"

#include "../Interfaces.h"
#include "../Memory.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/GameMovement.h"
#include "../SDK/MoveData.h"
#include "../SDK/MoveHelper.h"
#include "../SDK/Prediction.h"
#include "../SDK/MD5.h"

float previousCurrenttime;
float previousFrametime;

void PredictionSystem::StartPrediction(UserCmd* cmd) noexcept
{
    const auto localPlayer{ interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer()) };
    if (!localPlayer || !cmd)
        return;

    *memory.predictionRandomSeed = MD5::PseudoRandom(cmd->commandNumber) & 0x7FFFFFFF;
    **memory.predictionPlayer = localPlayer;

    previousCurrenttime = memory.globalVars->currenttime;
    previousFrametime = memory.globalVars->frametime;

    memory.globalVars->currenttime = memory.globalVars->serverTime(cmd);
    memory.globalVars->frametime = memory.globalVars->intervalPerTick;

    static MoveData moveData;
    memory.moveHelper->SetHost(localPlayer);
    interfaces.gameMovement->StartTrackPredictionErrors(localPlayer);
    interfaces.prediction->SetupMove(localPlayer, cmd, memory.moveHelper, &moveData);
    interfaces.gameMovement->ProcessMovement(localPlayer, &moveData);
    interfaces.prediction->FinishMove(localPlayer, cmd, &moveData);
}

void PredictionSystem::EndPrediction() noexcept
{
    const auto localPlayer{ interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer()) };

    interfaces.gameMovement->FinishTrackPredictionErrors(localPlayer);
    memory.moveHelper->SetHost(nullptr);

    *memory.predictionRandomSeed = -1;
    **memory.predictionPlayer = nullptr;

    memory.globalVars->currenttime = previousCurrenttime;
    memory.globalVars->frametime = previousFrametime;
}