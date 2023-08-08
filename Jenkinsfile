pipeline {
    agent any

    enviroment {
        CHECK_DEFINES = ''
        CHECK_ADDON = ''
        CHECK_CONF = 'CppCheck/suppressions.conf'
        SRC_DIR = 'Firmware'
    }

    stages
    {
        stage("cpp-check")
        {
            steps {
                    sh "cppcheck    ${CHECK_DEFINES}
                                    ${CHECK_ADDON}
                                    --enable=all
                                    --language=c
                                    --suppressions-list=${CHECK_CONF}
                                    --inline-suppr
                                    -q
                                    --xml --xml-version=2
                                    ${SRC_DIR}
                                    2> ${env.BRANCH_NAME}/${env.BUILD_ID}_report.xml"
            }
        }
        stage("doxygen")
        {
            steps {
                echo "Branch ${env.BRANCH_NAME}"
                echo "Running ${env.BUILD_ID} on ${env.JENKINS_URL}"
                dir("Doxygen")
                {
                    sh "doxygen config_doxygen"
                }
                publishHTML (target : [allowMissing: false,
                alwaysLinkToLastBuild: false,
                keepAll: false,
                reportDir: 'Doxygen/doxygen_output/html',
                reportFiles: 'index.html',
                reportName: 'Sailwind_Firmware_Documentation',
                reportTitles: 'Sailwind Firmware'])
            }
        }
    }
}