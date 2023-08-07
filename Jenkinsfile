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
                    sh "mkdir output"
                    sh "doxygen config_doxygen"
                }
            }
        }
    }
}