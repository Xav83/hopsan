#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QColor>
#include <QList>
#include <QFileInfo>
#include <QPen>
#include <QPalette>
#include <QFont>
#include <QDomElement>

class Configuration : public QObject
{
    Q_OBJECT

public:
    Configuration(QWidget *pParent);

    void loadFromXml();
    void loadDefaultsFromXml();

public slots:
    void saveToXml();
    bool getAlwaysSaveBeforeCompiling() const;
    QString getProjectPath() const;
    QString getHopsanPath() const;
    QString getIncludePath() const;
    QString getHopsanCoreLibPath() const;
    QString getCompilerPath() const;
    bool getUseTextWrapping() const;
    QPalette getPalette();
    QString getStyleSheet();

    void setAlwaysSaveBeforeCompiling(const bool &value);
    void setProjectPath(const QString &value);
    void setHopsanPath(const QString &value);
    void setCompilerPath(const QString &value);
    void setUseTextWrapping(const bool &value);

signals:
    void configChanged();

private:
    QWidget *mpParent;

    void loadUserSettings(QDomElement &rDomElement);
    void loadStyleSettings(QDomElement &rDomElement);

    bool mAlwaysSaveBeforeCompiling;
    QString mProjectPath;
    QString mHopsanPath;
    QString mCompilerPath;
    bool mUseTextWrapping;
    QPalette mPalette;
    QString mStyleSheet;
};

#endif // CONFIGURATION_H
