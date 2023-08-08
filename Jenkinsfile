pipeline {
    agent any

    environment {
        CHECK_CONF = 'CppCheck/suppressions.conf'
        SRC_DIR = 'Firmware'
    }

    stages
    {
        stage("cpp-check")
        {
            steps {
                    sh """cppcheck  --enable=all --language=c --suppressions-list=${CHECK_CONF} --inline-suppr -q --xml --xml-version=2 ${SRC_DIR} 2> report.xml"""
                    recordIssues(enabledForFailure: true, aggregatingResults: true, tool: cppCheck(pattern: 'report.xml'))
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