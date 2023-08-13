pipeline {
    agent any

    environment {
        CHECK_CONF = 'CppCheck/suppressions.conf'
        CHECK_CONF_MISRA = 'CppCheck/suppressions_misra.json'
        SRC_DIR = 'Firmware'
        HTML_FILES = 'index.html'
    }

    stages
    {
        stage("cpp-check")
        {
            steps {
                    sh "cppcheck  --enable=all --language=c --suppressions-list=${CHECK_CONF} --inline-suppr -q --xml --xml-version=2 ${SRC_DIR} 2> report.xml"
                    recordIssues tools: [cppCheck(pattern: 'report.xml')]
            }
        }
        stage("doxygen")
        {
            steps {
                echo "Branch ${env.BRANCH_NAME}"
                echo "Running ${env.BUILD_ID} on ${env.JENKINS_URL}"

                sh "doxygen Doxygen/config_doxygen"
                HTML_FILES = sh "find -name 'Doxygen/doxygen_output/html/*.html'"
                publishHTML([allowMissing: false, 
                alwaysLinkToLastBuild: true, 
                keepAll: true, 
                reportDir: 'Doxygen/doxygen_output/html', 
                reportFiles: '${env.HTML_FILES}', 
                reportName: 'HTML Report', 
                reportTitles: '', 
                useWrapperFileDirectly: true])
            }
        }
    }
}