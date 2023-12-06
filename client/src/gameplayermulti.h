#ifndef GAMEPLAYERMULTI_H
#define GAMEPLAYERMULTI_H

#include "gameplayer.h"

class GamePlayerMulti : public GamePlayer
{
    Q_OBJECT

    Q_PROPERTY(bool isSelf READ isSelf WRITE setIsSelf NOTIFY isSelfChanged FINAL)

public:
    GamePlayerMulti(QQuickItem *parent = nullptr);
    virtual ~GamePlayerMulti() {}


    static ObjectStateBase createState(const QPointF &pos);

    virtual void onTimingTimerTimeoutMulti(const bool &hosted, const int &msec, const qreal &delayFactor) override;
    virtual void cacheCurrentState() override;

    virtual void setStateFromSnapshot(const ObjectStateBase &ptr, const qint64 &currentTick, const bool &force) override;
    virtual ObjectStateBase getCurrentState() const override;
    virtual void setCurrentState(const ObjectStateBase &state, const bool &force) override;

    virtual void init(ActionGame *game) override;

    bool isSelf() const;
    void setIsSelf(bool newIsSelf);

signals:
    void isSelfChanged();

protected:
    virtual ObjectStateBase interpolate(const qreal &t, const ObjectStateBase &from, const ObjectStateBase &to) override;
    virtual void performAttack() override;

private:
    static const QHash<ObjectStateBase::PlayerState, PlayerState> m_stateHash;

    bool m_isSelf = false;
};

#endif // GAMEPLAYERMULTI_H
