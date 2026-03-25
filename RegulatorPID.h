#ifndef REGULATORPID_H
#define REGULATORPID_H

class RegulatorPID {
public:
    //enum do wyboru trybu liczenia ca�ki
    enum class LiczCalk { Zew, Wew };
    struct last_pid_values {
        double p,i,d;
    };

private:
    //nastawy regulatora
    double k;
    double Ti;
    double Td;

    last_pid_values _pidValues;

    //suma uchyb�w:
    double sumError;
    //poprzedni uchyb potrzebny dla nastawy r�niczkowej
    double prevError;
    //tryb liczenia ca�ki (zewn�trzny/wewn�trzny)
    LiczCalk liczCalk;

    //	Metody licz�ce poszczeg�lne sk�adowe regulatora
    double calcP(double k) const;
    double calcI(double e);
    double calcD(double e);
public:
    // Konstruktory:
    RegulatorPID(double k, double Ti = 0.0, double Td = 0.0);

    // Metoda symuluj�ca dzia�anie regulatora PID przy podanym uchybie e
    double simulate(double e);

    // Metody ustawiaj�ce parametry regulatora:
    void setK(double k);
    void setTi(double Ti);
    void setTd(double Td);
    void setLiczCalk(LiczCalk mode);
    void setStalaCalk(double Ti);

    const last_pid_values &getLastPidValues() const;

    //pod przycisk w gui reset pamieci calki
    void resetIntegral();

    // Metoda resetuj�ca stan wewn�trzny regulatora
    void reset();
};


#endif // REGULATORPID_H
