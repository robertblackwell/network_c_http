# Verify_app

The purpose of this app is to load test multi-threaded server, like `sync_app` and `async_app` in this project.

The app runs a number of indenpendent threads.

Each thread sends a number of `http` GET request to the server under test, verifies that the response has the expected 
body content and logs the response time.

The exact target or url in the get request may depend on the server and hence that is a command line argument
to the verify app.

When the test is over (all threads have sent their quota of requests) the app prints response time statistics. 