pipeline {
    agent any

    stages
    {

        stage("build")
        {
            steps {
                echo "building debug"
                echo "building release"
            }
        }
        stage("code-check")
        {
            steps {
                echo "cppcheck"
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
                reportDir: '/Doxygen/doxygen_output',
                reportFiles: 'index.html',
                reportName: 'Sailwind_Firmware_Documentation',
                reportTitles: 'Sailwind Firmware'])
            }
        }
    }
}