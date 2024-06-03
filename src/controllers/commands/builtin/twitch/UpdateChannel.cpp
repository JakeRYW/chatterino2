#include "controllers/commands/builtin/twitch/UpdateChannel.hpp"

#include "common/Channel.hpp"
#include "common/network/NetworkResult.hpp"
#include "controllers/commands/CommandContext.hpp"
#include "messages/MessageBuilder.hpp"
#include "providers/twitch/api/Helix.hpp"
#include "providers/twitch/TwitchChannel.hpp"

namespace chatterino::commands {

QString setTitle(const CommandContext &ctx)
{
    if (ctx.channel == nullptr)
    {
        return "";
    }

    if (ctx.words.size() < 2)
    {
        ctx.channel->addMessage(
            makeSystemMessage("Usage: /settitle <stream title>"));
        return "";
    }

    if (ctx.twitchChannel == nullptr)
    {
        ctx.channel->addMessage(
            makeSystemMessage("Unable to set title of non-Twitch channel."));
        return "";
    }

    auto title = ctx.words.mid(1).join(" ");

    getHelix()->updateChannel(
        ctx.twitchChannel->roomId(), "", "", title,
        [channel{ctx.channel}, title](const auto &result) {
            (void)result;

            channel->addMessage(
                makeSystemMessage(QString("Updated title to %1").arg(title)));
        },
        [channel{ctx.channel}](auto error, auto message) {
            QString errorMessage = QString("Failed to set title - ");

            using Error = HelixUpdateChannelError;

            switch(error)
            {
                case Error::UserMissingScope: {
                    errorMessage += "Missing required scope. "
                                    "Re-login with your "
                                    "account and try again.";
                }
                break;

                case Error::UserNotAuthorized: {
                    errorMessage += "You must be the broadcaster "
                                    "to set the title.";
                }
                break;

                case Error::Ratelimited: {
                    errorMessage +=
                        "You are being ratelimited by Twitch. Try "
                        "again in a few seconds.";
                }
                break;

                case Error::Forwarded: {
                    errorMessage += message;
                }
                break;

                case Error::Unknown:
                default: {
                    errorMessage += "An unknown error has occured.";
                }
                break;
            }

            channel->addMessage(makeSystemMessage(errorMessage));
        });

    return "";
}

QString setGame(const CommandContext &ctx)
{
    if (ctx.channel == nullptr)
    {
        return "";
    }

    if (ctx.words.size() < 2)
    {
        ctx.channel->addMessage(
            makeSystemMessage("Usage: /setgame <stream game>"));
        return "";
    }

    if (ctx.twitchChannel == nullptr)
    {
        ctx.channel->addMessage(
            makeSystemMessage("Unable to set game of non-Twitch channel."));
        return "";
    }

    const auto gameName = ctx.words.mid(1).join(" ");

    getHelix()->searchGames(
        gameName,
        [channel{ctx.channel}, twitchChannel{ctx.twitchChannel},
         gameName](const std::vector<HelixGame> &games) {
            if (games.empty())
            {
                channel->addMessage(makeSystemMessage("Game not found."));
                return;
            }

            auto matchedGame = games.at(0);

            if (games.size() > 1)
            {
                // NOTE: Improvements could be made with 'fuzzy string matching' code here
                // attempt to find the best looking game by comparing exactly with lowercase values
                for (const auto &game : games)
                {
                    if (game.name.toLower() == gameName.toLower())
                    {
                        matchedGame = game;
                        break;
                    }
                }
            }

            auto status = twitchChannel->accessStreamStatus();
            getHelix()->updateChannel(
                twitchChannel->roomId(), matchedGame.id, "", "",
                [channel, games, matchedGame](const NetworkResult &) {
                    channel->addMessage(makeSystemMessage(
                        QString("Updated game to %1").arg(matchedGame.name)));
                },
                [channel](auto error, auto message) {
                    QString errorMessage = QString("Failed to set game - ");

                    using Error = HelixUpdateChannelError;

                    switch(error)
                    {
                        case Error::UserMissingScope: {
                            errorMessage += "Missing required scope. "
                                            "Re-login with your "
                                            "account and try again.";
                        }
                        break;

                        case Error::UserNotAuthorized: {
                            errorMessage += "You must be the broadcaster "
                                            "to set the game.";
                        }
                        break;

                        case Error::Ratelimited: {
                            errorMessage +=
                                "You are being ratelimited by Twitch. Try "
                                "again in a few seconds.";
                        }
                        break;

                        case Error::Forwarded: {
                            errorMessage += message;
                        }
                        break;

                        case Error::Unknown:
                        default: {
                            errorMessage += "An unknown error has occured.";
                        }
                        break;
                    }

                    channel->addMessage(makeSystemMessage(errorMessage));
                });
        },
        [channel{ctx.channel}] {
            channel->addMessage(makeSystemMessage("Failed to look up game."));
        });

    return "";
}

}  // namespace chatterino::commands
