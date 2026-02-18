const { execSync } = require('child_process');
require('dotenv').config();

try {
    if (!process.env.GH_TOKEN) {
        throw new Error('GH_TOKEN not found in the .env file');
    }

    console.log('Starting release process...');

    execSync('semantic-release --no-ci', {
        stdio: 'inherit',
        env: {
            ...process.env,
            GITHUB_TOKEN: process.env.GH_TOKEN,
            GH_TOKEN: process.env.GH_TOKEN
        }
    });

    console.log('Release completed successfully!');
} catch (error) {
    console.error('Error during the release process:', error.message);
    process.exit(1);
}