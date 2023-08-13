pipeline {
    agent any

    environment {
        CHECK_CONF = 'CppCheck/suppressions.conf'
        CHECK_CONF_MISRA = 'CppCheck/suppressions_misra.json'
        SRC_DIR = 'Firmware'
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
            environment {
                TEST = sh returnStdout: true, script: 'find -maxdepth 1 -name \'*.html\' -type f'
            }
            
            steps {
                dir('doxygen_output/html') {
                    withEnv(['TEST = sh returnStdout: true, script: \'find -maxdepth 1 -name \\\'*.html\\\' -type f\'']) {
                        echo "${env.TEST}"
                        publishHTML([allowMissing: false, 
                        alwaysLinkToLastBuild: true, 
                        keepAll: true, 
                        reportDir: '', 
                        reportFiles: "${env.TEST}", 
                        reportName: 'HTML Report', 
                        reportTitles: '', 
                        useWrapperFileDirectly: true])
                    }
                }
            }
        }
    }
}