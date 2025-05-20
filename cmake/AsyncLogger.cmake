include(FetchContent)
FetchContent_Declare(
    AsyncLogger
    GIT_REPOSITORY https://github.com/Yimura/AsyncLogger.git
    GIT_TAG 1137fde556c7c0a91d87256118b9f286b0d67a38
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(AsyncLogger)
