pipeline {
    agent any

    environment {
        CHECK_CONF = 'CppCheck/suppressions.conf'
        CHECK_CONF_MISRA = 'CppCheck/suppressions_misra.json'
        SRC_DIR = 'Firmware'
        TEST = "lol"
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
            }
        }
        stage("deploy doxygen")
        {      
            steps {
                dir('doxygen_output/html') {
                    publishHTML([allowMissing: false, 
                    alwaysLinkToLastBuild: true, 
                    keepAll: true, 
                    reportDir: "", 
                    reportFiles: "index.html", 
                    reportName: 'Sailwindfirmware_doc', 
                    reportTitles: '', 
                    useWrapperFileDirectly: true])
                }
            }
        }
    }
}