const express = require("express");
const crypto = require("node:crypto");
require("dotenv").config();

const app = express();

const TOKEN = process.env.SWITCHBOT_TOKEN;
const SECRET = process.env.SWITCHBOT_SECRET;
const DEVICE_ID = process.env.SWITCHBOT_DEVICE_ID;

app.get("/turnOff", async (req, res) => {
  try {
    // 認証情報を生成
    const t = Date.now().toString();
    const nonce = crypto.randomUUID();

    const sign = crypto
      .createHmac("sha256", SECRET)
      .update(TOKEN + t + nonce)
      .digest("base64");

    // SwitchBot APIを呼び出す
    const response = await fetch(
      `https://api.switch-bot.com/v1.1/devices/${DEVICE_ID}/commands`,
      {
        method: "POST",
        headers: {
          Authorization: TOKEN,
          sign,
          nonce,
          t,
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          command: "turnOff",
          parameter: "default",
          commandType: "command",
        }),
      }
    );

    const data = await response.json();

    if (!response.ok || data.statusCode !== 100) {
      return res.status(502).json({
        success: false,
        error: data.message || "SwitchBot API error",
      });
    }

    res.json({ success: true, message: "Turned off" });
  } catch (error) {
    console.error(error);
    res.status(500).json({ success: false });
  }
});

app.listen(3000, "127.0.0.1", () => {
  console.log("Server running on http://localhost:3000");
});
